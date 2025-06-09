#include <unistd.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#ifndef __DJGPP__
#include <sys/mman.h>
#endif

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

int test_ucode_flag, help_flag;

#ifndef __DJGPP__
int fd;
#endif

void rdmsr( uint32_t msra , uint32_t *opt ) {
    uint64_t val;
#ifdef __DJGPP__
    asm volatile ( "rdmsr\n" : "=A" (val) : "c" (msra) );
#else
    if (pread(fd, &val, sizeof(val), msra) != sizeof(val)) {
        perror("msr read:");
        exit(127);
    }
#endif
    opt[0] = val & 0xffffffffu;
    val = val >> 32;
    opt[1] = val & 0xffffffffu;

}

void wrmsr( uint32_t msra , uint32_t lo, uint32_t hi ) {
#ifdef __DJGPP__
    asm volatile ("wrmsr\n" :  : "c"(msra), "a"(lo), "d"(hi) : "memory") ;
#else
    uint64_t val = hi;

    val = val << 32;
    val |= lo;

    if (pwrite(fd, &val, sizeof(val), msra) != sizeof(val)) {
        perror("msr write:");
        exit(127);
    }
#endif
}

void cpuid( uint32_t leaf, cpuid_opt_t *opt ) {
    uint32_t a,b,c,d;

    a = leaf;
    __asm__ volatile ("cpuid" : "+a" (a),"=b" (b), "=c" (c), "=d" (d));

    opt->r_eax = a;
    opt->r_ebx = b;
    opt->r_ecx = c;
    opt->r_edx = d;
}

uint32_t get_patchlvl() {
    cpuid_opt_t opt;
    uint32_t msrv[2];
    wrmsr(0x8b, 0, 0);
    cpuid(1, &opt);
    rdmsr(0x8b, msrv);
    return msrv[1];
}


void cpuinfo(uint32_t *plat_id, cpuid_opt_t *cpu_id) {
    uint32_t msrv[2];
    char brand[13];
    cpuid_opt_t opt;
    uint32_t leafs, l;
    cpuid( 0, &opt );
    leafs = opt.r_eax;
    brand[12] = 0;
    memcpy( brand, &opt.r_ebx, 12 );
    cpuid( 1, &opt );
    *cpu_id = opt;
    printf("CPU: \"%s\" Family %x Model %x Stepping %x\n", 
        brand,
        (opt.r_eax >> 8) & 0xf,
        (opt.r_eax >> 4) & 0xf,
        (opt.r_eax >> 0) & 0xf );
    printf("CPUID Level: %i, Features: EBX: %08X ECX: %08X EDX: %08X\n",
            leafs, opt.r_ebx, opt.r_ecx, opt.r_edx );
    printf("Microcode revision: %08X\n", get_patchlvl() );
    rdmsr( 0x17, msrv );
    *plat_id = 1u << ((msrv[1] >> 18) & 7);
    printf("MSR 0x017 lo: %08X hi: %08X, platform ID: %02X\n", msrv[0], msrv[1], *plat_id);
}



void *load_patch( const char *fn) {
    FILE *file;
    size_t sz;
    void *patchbuf = malloc(8192);
    void *patch = (void *)(((uint32_t)(patchbuf + 0x1000))&0xFFFFF000);
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

    return patch;
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
    "\t\t\n");
}

void parse_args( int argc, char *const *argv, char **testucode) {
    char opt;
    while ( (opt = getopt( argc, argv, "ht:" )) != -1 ) {
        switch( opt ) {
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

int main(int argc, char* argv[])
{

    uint32_t msrv[2];
    char msr_file_name[64];
    int32_t plat_id;
    cpuid_opt_t cpu_id;
    uint32_t patchlin;
    char *fname;
    char *testucode = NULL;
    patch_hdr_t *hdr;

    parse_args(argc, argv, &testucode);

    if (optind == (argc - 1)) {
        fname = argv[optind++];
    } else {
        usage("Wrong parameter count");
        exit(127);
    }

#ifndef __DJGPP__
    fd = open("/dev/cpu/0/msr", O_RDWR);
    if (fd < 0) {
        perror("msr open:");
        exit(EXIT_FAILURE);
    }
#endif
        
    printf("\n--------------------- Before update -------------\n");    
    cpuinfo(&plat_id, &cpu_id);
    printf("\n----------------------  Do update  --------------\n");
    hdr = load_patch(fname);

    if ((hdr->proc_sig != cpu_id.r_eax) || (hdr->proc_flags != plat_id)) {
        printf("CPUID / Platform ID mismatch CPU has %08X / %02X patch has %08X / %02X\n",
                    cpu_id.r_eax, plat_id, hdr->proc_sig,hdr->proc_flags);
        exit(EXIT_FAILURE);
    }

    patchlin = (uint32_t)&hdr->udata[0];
    printf("loading patch at %08X...\n", patchlin);
    wrmsr( 0x79, patchlin, 0);
    printf("\n---------------------- After update -------------\n");
    cpuinfo(&plat_id, &cpu_id);

    if (test_ucode_flag) {
        int i;
        uint32_t newlevel = get_patchlvl();
        uint32_t testlevel;
        uint8_t tmp;
        patch_hdr_t *hdr1;
        uint32_t patchlin1;

        printf("Trying to corrupt %s update at all byte offsets!\n", testucode);
        hdr1 = load_patch(testucode);
        patchlin1 = (uint32_t)&hdr1->udata[0];

        for (i=0;i<2048;i++) {
            uint8_t *p = (uint8_t *) hdr1;
            printf("Load on corrupt offset: %04x ", i);
            tmp = p[i];
            /* make sure byte is changed */
            p[i]++;
            wrmsr( 0x79, patchlin1, 0);
            testlevel = get_patchlvl();
            if (testlevel != newlevel) {
                 printf("OK\n");
            } else {
                 printf("FAILED\n");
            }
            wrmsr( 0x79, patchlin, 0);
            p[i] = tmp;
        }
    }

    return 0;
}
