#include <unistd.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

typedef struct {
    uint32_t r_eax;
    uint32_t r_ebx;
    uint32_t r_edx;
    uint32_t r_ecx;
} cpuid_opt_t;


int fd;

void rdmsr( uint32_t msra , uint32_t *opt ) {
    uint64_t val;
    if (pread(fd, &val, sizeof(val), msra) != sizeof(val)) {
        perror("msr read:");
        exit(127);
    }
    opt[0] = val & 0xffffffffu;
    val = val >> 32;
    opt[1] = val & 0xffffffffu;
}

void wrmsr( uint32_t msra , uint32_t lo, uint32_t hi ) {
    uint64_t val = hi;

    val = val << 32;
    val |= lo;

    if (pwrite(fd, &val, sizeof(val), msra) != sizeof(val)) {
        perror("msr write:");
        exit(127);
    }
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


void cpuinfo() {
    uint32_t msrv[2];
    char brand[13];
    cpuid_opt_t opt;
    uint32_t leafs, l;
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
    rdmsr( 0x17, msrv );
    printf("MSR 0x017: %08X %08X\n", msrv[0], msrv[1] );
}

char patchbuf[4096];

uint32_t *patch;

void load_patch( const char *fn ) {
    FILE *file;
    size_t sz;
    uint32_t patchlin;
    patch = (uint32_t *)(((uint32_t)(patchbuf + 0x100))&0xFFFFFF00);
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
    patchlin = (uint32_t)patch;
    printf("loading patch at %08X...\n", patchlin);
    wrmsr( 0x79, patchlin + 0x30 , 0);
    printf("still alive!\n");
}

int main(int argc, char* argv[])
{

    uint32_t msrv[2];
    char msr_file_name[64];

    if (argc < 2) {
            printf("need patch file name!\n");
        exit(127);
    }

    fd = open("/dev/cpu/0/msr", O_RDWR);
    if (fd < 0) {
        perror("msr open:");
        exit(127);
    }

        
    printf("\n--------------------- Before update -------------\n");    
    cpuinfo();
    printf("\n----------------------  Do update  --------------\n");
    load_patch(argv[1]);
    printf("\n---------------------- After update -------------\n");
    cpuinfo();
    return 0;
}
