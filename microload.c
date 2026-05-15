#ifdef __linux__
#define _GNU_SOURCE
#include <sched.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include <ucontext.h>
#endif
#include <setjmp.h>
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
#include <signal.h>
#include <dpmi.h>
#include <sys/segments.h>
#include <sys/exceptn.h>
#else
#include <sys/mman.h>
#endif


#define DEVMSR "/dev/cpu/0/msr"
#define UCODE_MAX_SIZE 0x1000

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


typedef struct rtable_entry
{
    uint32_t eip;
    uint32_t reeip;
} rtable_entry_t;

/* defined by linker */
extern rtable_entry_t _rtable_start;
extern rtable_entry_t _rtable_end;

#ifdef __DJGPP__

#define ADD_RTABLE(eip, reeip) \
    ".section .rtable,\"d\" \n" \
    ".balign 4 \n" \
    ".long " eip "," reeip "\n" \
    ".section .text\n"
#else
#define ADD_RTABLE(eip, reeip) \
    ".section .rtable,\"a\" \n" \
    ".balign 4 \n" \
    ".long " eip "," reeip "\n" \
    ".previous\n"
#endif

struct sigaction old_sigsegv;
struct sigaction old_sigill;

int test_ucode_flag, help_flag, dump_ucoderom_flag, dump_crom_flag, find_exec_flag;
int fd;

#ifdef __DJGPP__
static void sighandler(int mysignal)
{
#else
static void sighandler(
    __attribute__ ((unused)) int mysignal,
    __attribute__ ((unused)) siginfo_t *si,
    void* arg)
{
    ucontext_t *context = (ucontext_t *)arg;
#endif
    struct sigaction dfl = {0};

    rtable_entry_t *iter = &_rtable_start;

    while (iter < &_rtable_end) {

#ifdef __DJGPP__
        if (__djgpp_exception_state->__eip == iter->eip) {
            __djgpp_exception_state->__eip = iter->reeip;
            longjmp(__djgpp_exception_state, 0);
        }
#else
        if (context->uc_mcontext.gregs[REG_EIP] == (greg_t) iter->eip) {
            context->uc_mcontext.gregs[REG_EIP] = (greg_t) iter->reeip;
            return;
        }
#endif
        iter++;
    }
    /* Attempt to call default signal handler */
    dfl.sa_handler = SIG_DFL;
    sigaction(mysignal, &dfl, NULL);
    raise(mysignal);
}

static int rdpmc( uint32_t pctr_arg, uint32_t *opt ) {
    uint32_t lo, hi;
    int err = 0;
        __asm volatile (
            "1:rdpmc\n"
            "jmp 3f\n"
            "2:mov $1, %0\n"
            "3:\n"
            ADD_RTABLE("1b", "2b")
            : "=r" (err), "=a" (lo), "=d" (hi)
            : "c" (pctr_arg)
        );
    opt[0] = lo;
    opt[1] = hi;
    return err;
}

static int rdmsr( uint32_t msra , uint32_t *opt ) {
    uint64_t val = 0;
    int ret = 0;
#ifdef __DJGPP__
    asm volatile ( "1: rdmsr\n"
                   "jmp 3f\n"
                   "2:mov $1, %1\n"
                   "3:\n"
                   ADD_RTABLE("1b", "2b")
                   : "=A" (val), "+r" (ret)
                   : "c" (msra) );
#else
    if (pread(fd, &val, sizeof(val), msra) != sizeof(val)) {
        ret = 1;
    }
#endif
    opt[0] = val & 0xffffffffu;
    val = val >> 32;
    opt[1] = val & 0xffffffffu;
    return ret;
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

    int ret = 0;
#ifdef __DJGPP__
    uint32_t lo, hi;
    lo = val & 0xffffffffu;
    hi = val >> 32;

    asm volatile ("1:wrmsr\n"
                   "jmp 3f\n"
                   "2:mov $1, %0\n"
                   "3:\n"
                   ADD_RTABLE("1b", "2b")
                   :  "+r" (ret)
                   : "c"(msra), "a"(lo), "d"(hi) : "memory") ;
#else
    if (pwrite(fd, &val, sizeof(val), msra) != sizeof(val)) {
        assert(!"WRMSR faulted");
        ret = 1;
    }
#endif
    return ret;
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
    p+= align;
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
    void *patch = malloc_aligned(UCODE_MAX_SIZE, 4096);
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
    sz = fread( patch, 1, UCODE_MAX_SIZE , file );
    fclose( file );
    if ( sz < 2048 ) {
        fprintf( stderr, "short read! %i\n", sz );
    }
#ifndef __DJGPP__
    if (mlock(patch, UCODE_MAX_SIZE) == -1) {
        perror("Locking memory failed!");
        exit( EXIT_FAILURE );
    }
#endif
    /* If plat_id exists, allow multihit of plat_id */
    if ((hdr->proc_sig != opt.r_eax) ||
        (((hdr->proc_flags & plat_id) == 0) && (plat_id != 0))) {
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
    "\tmicroload [-h] [-d] [-c] [-f] [-t <testfilename>] <filename> \n\n" );

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
    "\t\t-c                Dump the microcode constant ROM to RAM using a special\n"
    "\t\t                  microcode update. Provide the file prefix as a filename\n"
    "\t\t-f                Perform a debug tracing over all files with specific prefix\n"
    "\t\t                  provided as a filename\n"

    "\t\t\n");
}

static void parse_args( int argc, char *const *argv, char **testucode) {
    int opt;
    while ( (opt = getopt( argc, argv, "hfdct:" )) != -1 ) {
        switch( opt ) {
            case 'd':
                dump_ucoderom_flag = 1;
                break;
            case 'c':
                dump_crom_flag = 1;
                break;
            case 'f':
                find_exec_flag = 1;
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
    int j, r, num_ucode_dw;
    patch_hdr_t *hdr;
    uint32_t msrv[2];
    uint32_t size, addr, dw_per_triplet;
    uint32_t *ucode;
    uint64_t patchlin;

    num_ucode_dw = NUM_UCODE_DW;
    size = (NUM_UCODE_DW  + 1) * sizeof(uint32_t);
    ucode = malloc_aligned(size, 32);

    uint64_t ucode_lin = get_lin_addr(ucode);

    memset(ucode, TEST_FILL, size);

    hdr = load_patch(fname);
    patchlin = get_lin_addr(&hdr->udata[0]);
    dw_per_triplet = 8;

    if (hdr->proc_sig < 0x630) {
        dw_per_triplet = 7;
        num_ucode_dw = 0x7000;
        assert(num_ucode_dw < NUM_UCODE_DW);
    }

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

    if (ucode[0x1000] == TEST_FILL) {
        uint32_t rdpmcv[2];
        printf("Fail, but trying to re-trigger it using RDPMC\n");
        rdpmc(0, rdpmcv);
    }

    rdmsr(IA32_BIOS_SIGN_ID, msrv);
    printf("MSR IA32_BIOS_SIGN_ID is %x %x\n", msrv[1], msrv[0]);

    if (ucode[0x1000] == TEST_FILL) {
        printf("Fail :(\n");
        return;
    }

    addr = 0;
    for (j = 0;j < num_ucode_dw;j++) {
        if (((j % dw_per_triplet) == 0)) {
            printf("\n%04X:", addr);
            addr += 8;
        }
        printf(" %08X", ucode[j]);
    }
    printf("\n");
}

#ifdef DEBUG_TRACING
uint32_t xxx;
#endif

static void find_exec(char *fname)
{
    char tmp_buff[128];
    int i, r;
    cpuid_opt_t opt;
    patch_hdr_t *hdr;
    uint32_t msrv[2];
    uint64_t patchlin;
    /* Use every 4th microcode address for now */
    for (i=0;i<0x3800;i+=4) {

        /* Invalid address */
        if ((i % 4) == 3) {
            continue;
        }

        snprintf(tmp_buff, sizeof(tmp_buff), "%s-%04x.dat", fname, i);
//      printf("Trying to breakpoint on cpuid at %s\n", tmp_buff);

        hdr = load_patch(tmp_buff);
        patchlin = get_lin_addr(&hdr->udata[0]);

        wrmsr(IA32_BIOS_SIGN_ID, 0xdeadbeefaffecafe);
        wrmsr(IA32_BIOS_UPDT_TRIG, patchlin);
#ifdef DEBUG_TRACING
        uint32_t rdpmcv[2];
/* Maybe those can be later auto-discovered in the microcode */
/* WARNING: inline assembly constrains are WRONG */

//      __asm__ volatile ("mov %%cs, %%ax \nlar %%ax, %%ax\n" : : : "memory", "eax", "cc");
//      __asm__ volatile ("mov %%cs, %%ax \nlsl %%ax, %%ax\n" : : : "memory", "eax", "cc");
//      __asm__ volatile ("mov %%cs, (%%eax) \nlar (%%eax), %%ax\n" : : "a" (&xxx) : "memory", "cc");
//      __asm__ volatile ("mov %%cs, (%%eax) \nlsl (%%eax), %%ax\n" : : "a" (&xxx) : "memory", "cc");
//      __asm__ volatile ("mov %%cs, %%ax \nlar %%ax, %%ax\n" : : : "memory", "eax", "cc");
//      __asm__ volatile ("push %%ds\npop %%fs" : : "a" (0x42424242) : "memory", "cc");
//      __asm__ volatile ("push %%es\npop %%ss" : : "a" (0x42424242) : "memory", "cc");
//      __asm__ volatile ("pushf \npop %%eax\n" : : "a" (0x42424242) : "memory", "cc");
//      __asm__ volatile ("smsw %%eax\n" : : "a" (0x42424242) : "memory", "cc");

//      __asm__ volatile ("bts %%bx, (%%ebx)\n" : : "a" (0x42424242), "b" (&xxx) : "memory", "cc");
//      __asm__ volatile ("lfs (%%ebx), %%ebx\n" : : "a" (0x42424242), "b" (&xxx) : "memory", "cc");

//      __asm__ volatile ("mov %%cs, %%bx \nverw %%bx\n" :  :  "a" (0x42424242) : "memory", "cc", "ebx");
//      __asm__ volatile ("mov %%cs, %%bx \nverr %%bx\n" :  :  "a" (0x42424242) : "memory", "cc", "ebx");
//      __asm__ volatile ("mov %%ds, (%%eax) \nmov (%%eax), %%fs\n" : : "a" (&xxx) : "memory", "cc");
//      rdpmc(0, rdpmcv);
#endif
        /* For now look for CPUID leaf 1 */
        cpuid(1, &opt);
        r = rdmsr(IA32_BIOS_SIGN_ID, msrv);
        assert(r == 0);
        if (msrv[1] != 0xdeadbeef) {
            printf("ucode trace hit with %s, %x %x\n", tmp_buff, msrv[0], msrv[1]);
        } else {
//            printf("%x %x\n", msrv[0], msrv[1]);
        }
    }
}

#define NUM_CROM_QW (512)

static void dump_crom(char *fname)
{
    int i, j, r;
    char tmp_buff[128];
    patch_hdr_t *hdr;
    uint32_t msrv[2];
    uint32_t size;
    uint64_t *crom;
    uint64_t patchlin;


    size = (NUM_CROM_QW  + 1) * sizeof(uint64_t);
    crom = malloc_aligned(size, 32);

    memset(crom, TEST_FILL, size);

    for (i=0;i<(NUM_CROM_QW);i+=16) {

        uint64_t crom_lin = get_lin_addr(&crom[i]);


        snprintf(tmp_buff, sizeof(tmp_buff), "%s-%d.dat", fname, i);

        hdr = load_patch(tmp_buff);
        patchlin = get_lin_addr(&hdr->udata[0]);

        /* communicate the buffer in hi bits of IA32_BIOS_SIGN_ID */
        wrmsr(IA32_BIOS_SIGN_ID, crom_lin << 32);
        wrmsr(IA32_BIOS_UPDT_TRIG, patchlin);

        r = rdmsr(IA32_BIOS_SIGN_ID, msrv);
        assert(r == 0);
        wrmsr(IA32_BIOS_SIGN_ID, crom_lin << 32);
        cpuid_opt_t opt;
        cpuid(1, &opt);
        uint32_t rdpmcv[2];
        rdpmc(0, rdpmcv);
    }

    for (j = 0;j < NUM_CROM_QW;j++) {
        printf("%03X: %016LX\n", j, crom[j]);
    }
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

    for (i=0;i<UCODE_MAX_SIZE;i++) {
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

#ifdef __DJGPP__
/* COFF lacks .previous directive */
__attribute__((section(".text")))
#endif
int main(int argc, char* argv[])
{
    char *fname;
    char *testucode = NULL;

#ifdef DEBUG_RTABLE
  rtable_entry_t *iter = &_rtable_start;

    while (iter < &_rtable_end) {
         printf("REC: %x -> %x\n", iter->eip, iter->reeip);
        iter++;
    }
#endif

    parse_args(argc, argv, &testucode);

    if (optind == (argc - 1)) {
        fname = argv[optind++];
    } else {
        usage("Wrong parameter count");
        exit(127);
    }

    assert(fname != NULL);
#ifdef __DJGPP__
    signal(SIGSEGV, sighandler);
    /* DOSBOX emulates WRMSR fault as SIGILL */
    signal(SIGILL, sighandler);
#else
    struct sigaction action = { 0 };
    action.sa_sigaction = &sighandler;
    action.sa_flags = SA_SIGINFO;
    sigaction(SIGSEGV, &action, NULL);
    sigaction(SIGILL, &action, NULL);

    fd = open(DEVMSR, O_RDWR);
    if (fd < 0) {
        perror(DEVMSR " open");
        exit(EXIT_FAILURE);
    }

    /* Currently only CPU 0 is supported, set us there */
    {
        cpu_set_t set;
        CPU_ZERO(&set);
        CPU_SET(0, &set);
        if (sched_setaffinity(getpid(), sizeof(set), &set) == -1) {
            perror("Failed to set affinity to CPU 0");
            exit(EXIT_FAILURE);
        }
    }
#endif

    if (dump_ucoderom_flag) {
        dump_ucoderom(fname);
    } else if (test_ucode_flag) {
        assert(testucode != NULL);
        test_ucode_structure(fname, testucode);
    } else if (dump_crom_flag) {
        dump_crom(fname);
    } else if (find_exec_flag) {
        find_exec(fname);
    } else {
        update_ucode(fname);
    }

    return 0;
}
