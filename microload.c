#include <unistd.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <malloc.h>
#include <assert.h>
#ifdef __DJGPP__
#include <setjmp.h>
#include <signal.h>
#include <dpmi.h>
#include <sys/segments.h>
#include <sys/exceptn.h>
#else
#include <sys/mman.h>
#endif

#define DEVMSR "/dev/cpu/0/msr"

#define IA32_PLATFORM_ID 0x17
#define IA32_BIOS_SIGN_ID 0x8b
#define IA32_BIOS_UPDT_TRIG 0x79

/* Size of ucode in dwords */
#define NUM_UCODE_DW 0x8000
#define TEST_FILL 0xaaaaaaaa

typedef struct {
    uint32_t r_eax;
    uint32_t r_ebx;
    uint32_t r_edx;
    uint32_t r_ecx;
} cpuid_opt_t;

typedef struct __attribute__((packed)) {
    uint32_t      header_ver;
    uint32_t      update_rev;
    uint32_t      date_bcd;
    uint32_t      proc_sig;
    uint32_t      checksum;
    uint32_t      loader_ver;
    uint32_t      proc_flags;
    uint32_t      data_size;
    uint32_t      total_size;
    uint8_t       reserved[12];
    uint8_t       udata[0];
} patch_hdr_t;

int test_ucode_flag, help_flag, dump_ucoderom_flag;

#ifdef __DJGPP__
jmp_buf wrmsr_gpf;
jmp_buf rdmsr_gpf;
/* hack to get rdmsr/wrmsr instruction address */
extern void rdmsr_eip(void);
extern void wrmsr_eip(void);

static void sighandler(int s)
{
    if (__djgpp_exception_state->__eip == ((uint32_t) &rdmsr_eip)) {
        /* recover the RDMSR */
        longjmp(rdmsr_gpf, 1);
    } else if (__djgpp_exception_state->__eip == ((uint32_t) &wrmsr_eip)) {
        /* recover the WRMSR */
        longjmp(wrmsr_gpf, 1);
    }
    /* let default handler run */
}
#else
int fd;
#endif

static int rdmsr( uint32_t msra , uint32_t *opt ) {
    uint64_t val;
#ifdef __DJGPP__
    if (setjmp(rdmsr_gpf) == 0) {
        asm volatile ( "_rdmsr_eip: rdmsr\n" : "=A" (val) : "c" (msra) );
    } else {
        return 1;
    }
#else
    if (pread(fd, &val, sizeof(val), msra) != sizeof(val)) {
        return 1;
    }
#endif
    opt[0] = val & 0xffffffffu;
    val = val >> 32;
    opt[1] = val & 0xffffffffu;
    return 0;
}

static inline uint64_t rdtsc(void)
{
    uint32_t low, high;
    /* hack to execute cpuid with leaf 0 */
    low = 0;
    asm volatile("cpuid\n" /* serialize */
                 "rdtsc\n" : "+a"(low), "=d"(high) :: "ebx", "ecx", "memory");
    return ((uint64_t)high << 32) | low;
}

static int wrmsr( uint32_t msra, uint64_t val) {
#ifdef __DJGPP__
    uint32_t lo, hi;
    lo = val & 0xffffffffu;
    hi = val >> 32;
    if (setjmp(wrmsr_gpf) == 0) {
        asm volatile ("_wrmsr_eip: wrmsr\n" :  : "c"(msra), "a"(lo), "d"(hi) : "memory") ;
    } else {
        assert(!"WRMSR faulted");
        return 1;
    }
#else
    if (pwrite(fd, &val, sizeof(val), msra) != sizeof(val)) {
        assert(!"WRMSR faulted");
        return 1;
    }
#endif
    return 0;
}

/* DJGPP has broken implementation of posix_memalign / memalign */
static void *malloc_aligned(size_t size, uintptr_t align)
{
    void *ret;
    uintptr_t p;
    
    ret = malloc(size + align);

    if (ret == NULL) {
        return ret;
    }

    p = (uintptr_t) ret;
    return (void *) (p & ~(align - 1));
}

static void cpuid( uint32_t leaf, cpuid_opt_t *opt ) {
    uint32_t a,b,c,d;

    a = leaf;
    __asm__ volatile ("cpuid" : "+a" (a),"=b" (b), "=c" (c), "=d" (d));

    opt->r_eax = a;
    opt->r_ebx = b;
    opt->r_ecx = c;
    opt->r_edx = d;
}

static uint32_t get_patchlvl() {
    int r;
    cpuid_opt_t opt;
    uint32_t msrv[2];
    wrmsr(IA32_BIOS_SIGN_ID, 0);
    cpuid(1, &opt);
    r = rdmsr(IA32_BIOS_SIGN_ID, msrv);
    assert(r == 0);
    return msrv[1];
}

static uint32_t get_platid(void) {
    uint32_t msrv[2];
    uint32_t ret;

    ret = 0;
    if (!rdmsr(IA32_PLATFORM_ID, msrv)) {
        ret = 1u << ((msrv[1] >> 18) & 7);
    } else {
        printf("WARNING: IA32_PLATFORM_ID not available, assuming 0\n");
    }
    return ret;
}

static void print_cpuinfo(void) {
    char brand[13];
    cpuid_opt_t opt;
    uint32_t leafs;
    cpuid( 0, &opt );
    leafs = opt.r_eax;
    brand[12] = 0;
    memcpy( brand, &opt.r_ebx, 12 );
    cpuid( 1, &opt );
    printf("CPU: \"%s\" Family %x Model %x Stepping %x\n", 
        brand,
        (opt.r_eax >> 8) & 0xf,
        (opt.r_eax >> 4) & 0xf,
        (opt.r_eax >> 0) & 0xf );
    printf("CPUID Level: %i, Features: EBX: %08X ECX: %08X EDX: %08X\n",
            leafs, opt.r_ebx, opt.r_ecx, opt.r_edx );
    printf("Microcode revision: %08X\n", get_patchlvl() );
    printf("Platform ID: %02X\n", get_platid());
}

static patch_hdr_t *load_patch( const char *fn) {
    patch_hdr_t *hdr;
    FILE *file;
    size_t sz;
    void *patch = malloc_aligned(0x2000, 32);
    hdr = patch;
    assert(patch != NULL);
    uint32_t plat_id = get_platid();
    cpuid_opt_t opt;
    
    cpuid( 1, &opt );

    file = fopen( fn, "rb" );
    if ( !file ) {
        fprintf(stderr,"Could not open file %s\n", fn);
        exit( EXIT_FAILURE );
    }
    sz = fread( patch, 1, 2048, file );
    fclose( file );
    if ( sz < 2048 ) {
        fprintf( stderr, "short read! %i\n", sz );
    }
#ifndef __DJGPP__
    if (mlock(patch, 4096) == -1) {
        perror("Locking memory failed!");
        exit( EXIT_FAILURE );
    }
#endif

    if ((hdr->proc_sig != opt.r_eax) || (hdr->proc_flags != plat_id)) {
        printf("CPUID / Platform ID mismatch CPU has %08X / %02X patch has %08X / %02X\n",
                    opt.r_eax, plat_id, hdr->proc_sig,hdr->proc_flags);
        exit(EXIT_FAILURE);
    }

    return hdr;
}

void usage( const char *reason ) {
    fprintf( stderr, "%s\n", reason );
    fprintf( stderr,
    "\tmicroload -h\n" );
    fprintf( stderr,
    "\tmicroload  [-t <testfilename>] <filename> \n\n" );

    if ( !help_flag )
        exit( EXIT_FAILURE );

    fprintf( stderr,
    "\t\n"
    "\tProgram for loading a microcode update into Intel CPU\n"
    "\twritten by Peter Bosch <public@pbx.sh> and ported to Linux/DOS\n"
    "\tand extended by Rudolf Marek <r.marek@assembler.cz>\n"
    "\n"
    "\tThis program might crash or damage the system they are loaded onto\n"
    "\tand the authors takes no responsibility for any damages resulting  \n"
    "\tfrom use of the software.\n"
    "\t\t-h                Print this message and exit\n"
    "\t\t\n"
    "\t\t-t                Perform side channel attack on microcode update.\n"
    "\t\t                  The <testfilename> specifies second update to be loaded\n"
    "\t\t                  in which each byte will be test-corrupted\n"
    "\t\t                  and results will be printed\n"
    "\t\t                  To make it work each update must be different revision\n"
    "\t\t-d                Dump the microcode ROM to RAM using a special microcode\n"
    "\t\t                  update.\n"
    "\t\t\n");
}

static void parse_args( int argc, char *const *argv, char **testucode) {
    int opt;
    while ( (opt = getopt( argc, argv, "hdt:" )) != -1 ) {
        switch( opt ) {
            case 'd':
                dump_ucoderom_flag = 1;
                break;
            case 't':
                test_ucode_flag = 1;
                *testucode = optarg;
                break;
            case 'h':
                help_flag = 1;
                usage("");
                exit( EXIT_FAILURE );
            break;
            case ':':
                usage("missing argument");
                break;
            default:
            case '?':
                usage("unknown argument");
                break;
        }
    }
}

static uint64_t get_lin_addr(void *ptr)
{
    uint64_t ret;
    ret = (uintptr_t) ptr;
#ifdef __DJGPP__
    uint32_t base;
    /* The DJGPP uses non-zero segment bases and microcode update needs linear address */
    if ((__dpmi_get_segment_base_address(_my_ds(), &base)) != 0) {
        perror("Unable to get segment base");
        exit(EXIT_FAILURE);
    }
    ret += base;
#endif
    return ret;
}


static void dump_ucoderom(char *fname)
{
    int j, r;
    patch_hdr_t *hdr;
    uint32_t msrv[2];
    uint32_t size;
    uint32_t *ucode;
    uint64_t patchlin;

    size = (NUM_UCODE_DW  + 1) * sizeof(uint32_t);
    ucode = malloc_aligned(size, 32);

    uint64_t ucode_lin = get_lin_addr(ucode);

    memset(ucode, TEST_FILL, size);

    hdr = load_patch(fname);
    patchlin = get_lin_addr(&hdr->udata[0]);

    printf("loading patch from 0x%08LX, ucode will be at 0x%08LX...\n", patchlin, ucode_lin);
    /* communicate the buffer in hi bits of IA32_BIOS_SIGN_ID */
    wrmsr(IA32_BIOS_SIGN_ID, ucode_lin << 32);
    wrmsr(IA32_BIOS_UPDT_TRIG, patchlin);
    assert(ucode[NUM_UCODE_DW] == TEST_FILL);

    r = rdmsr(IA32_BIOS_SIGN_ID, msrv);
    assert(r == 0);
    printf("MSR IA32_BIOS_SIGN_ID is %x %x\n", msrv[1], msrv[0]);

    printf("Checking if ucode is dumped...\n");

    if (ucode[0x1000] == TEST_FILL) {
        printf("Fail, but trying to re-trigger it using WRMSR to 0x8b\n");
        wrmsr(IA32_BIOS_SIGN_ID, ucode_lin << 32);
    }

    if (ucode[0x1000] == TEST_FILL) {
        printf("Fail, but trying to re-trigger it using CPUID\n");
        cpuid_opt_t opt;
        cpuid(1, &opt);
    }

    rdmsr(IA32_BIOS_SIGN_ID, msrv);
    printf("MSR IA32_BIOS_SIGN_ID is %x %x\n", msrv[1], msrv[0]);

    if (ucode[0x1000] == TEST_FILL) {
        printf("Fail :(\n");
        return;
    }

    for (j = 0;j < NUM_UCODE_DW;j++) {
        if (((j % 8) == 0)) {
            printf("\n%04X:", j);
        }
        printf(" %08X", ucode[j]);
    }
    printf("\n");
}

static void test_ucode_structure(char *fname, char *testucode)
{
    int i;
    uint32_t testlevel, oldlevel;
    uint8_t tmp;
    patch_hdr_t *hdr0, *hdr1;
    uint64_t patchlin1, patchlin0;

    printf("Trying to corrupt %s update at all byte offsets!\n", testucode);
    hdr0 = load_patch(fname);
    hdr1 = load_patch(testucode);
    patchlin0 = get_lin_addr(&hdr0->udata[0]);
    patchlin1 = get_lin_addr(&hdr1->udata[0]);
    wrmsr(IA32_BIOS_UPDT_TRIG, patchlin0);

    oldlevel = get_patchlvl();
    printf("Current ucode version is %x\n", oldlevel);

    for (i=0;i<2048;i++) {
        uint8_t *p = (uint8_t *) hdr1;
        uint64_t start, stop;
        printf("Load on corrupt offset: %04x ", i);
        tmp = p[i];
        /* make sure byte is changed */
        p[i]++;
        start = rdtsc();
        wrmsr(IA32_BIOS_UPDT_TRIG, patchlin1);
        stop = rdtsc();
        testlevel = get_patchlvl();
        if (testlevel != oldlevel) {
            printf("OK");
        } else {
            printf("FAILED");
        }
        printf(", took %lld cycles\n", (unsigned long long) stop - start);
        wrmsr(IA32_BIOS_UPDT_TRIG, patchlin0);
        p[i] = tmp;
    }
}

static void update_ucode(char *fname)
{
    uint64_t patchlin;
    patch_hdr_t *hdr;
    printf("\n--------------------- Before update -------------\n");    
    print_cpuinfo();
    printf("\n----------------------  Do update  --------------\n");
    hdr = load_patch(fname);

    patchlin = get_lin_addr(&hdr->udata[0]);

    printf("loading patch from 0x%08LX...\n", patchlin);

    wrmsr(IA32_BIOS_UPDT_TRIG, patchlin);
    printf("\n---------------------- After update -------------\n");

    print_cpuinfo();
}

int main(int argc, char* argv[])
{
    char *fname;
    char *testucode = NULL;

    parse_args(argc, argv, &testucode);

    if (optind == (argc - 1)) {
        fname = argv[optind++];
    } else {
        usage("Wrong parameter count");
        exit(127);
    }

#ifdef __DJGPP__
    signal(SIGSEGV, sighandler);
    /* DOSBOX emulates WRMSR fault as SIGILL */
    signal(SIGILL, sighandler);
#else
    fd = open(DEVMSR, O_RDWR);
    if (fd < 0) {
        perror(DEVMSR " open");
        exit(EXIT_FAILURE);
    }
#endif
    assert(fname != NULL);

    if (dump_ucoderom_flag) {
        dump_ucoderom(fname);
    } else if (test_ucode_flag) {
        assert(testucode != NULL);
        test_ucode_structure(fname, testucode);
    } else {
        update_ucode(fname);
    }

    return 0;
}
