// cpuid_ext_tools.cpp : Defines the entry point for the console application.
// written by @peterbjornx https://twitter.com/peterbjornx

#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <stdint.h>
#include <assert.h>

/* Command line flags */
int crbus_read_flag, crbus_write_flag, fprom_read_flag, crbus_read_index_flag, help_flag;

typedef struct {
    uint32_t r_eax;
    uint32_t r_ebx;
    uint32_t r_edx;
    uint32_t r_ecx;
} cpuid_opt_t;

void cpuid( uint32_t leaf, uint32_t a_ebx, uint32_t a_ecx, cpuid_opt_t *opt ) {
    uint32_t a,b,c,d;

    a = leaf;
/*    printf("CPUID: EAX %x EBX %x ECX %x\n", leaf, a_ebx, a_ecx); */
    __asm__ volatile ("cpuid" : "+a" (a),"+b" (a_ebx), "+c" (a_ecx), "=d" (d));
/*    printf("CPUID: EAX %x EBX %x ECX %x EDX %x\n", a, a_ebx, a_ecx, d); */
    opt->r_eax = a;
    opt->r_ebx = a_ebx;
    opt->r_ecx = a_ecx;
    opt->r_edx = d;
}

uint32_t cpuid_leaf0( char *brand ) {
    cpuid_opt_t opt;
    cpuid( 0, 0, 0, &opt );
    brand[12] = 0;
    memcpy( brand, &opt.r_ebx, 12 );
    return opt.r_eax;
}

uint32_t read_arr188( uint32_t addr ) {
    cpuid_opt_t opt;
    cpuid( 0x42, addr, 0, &opt );
    assert( opt.r_ecx == 0x42 );
    return opt.r_eax;
}

void movetocreg( uint32_t addr, uint32_t val ) {
    cpuid_opt_t opt;
    cpuid( 0x43, val, addr, &opt );
    assert( opt.r_ecx == 0x42 );
}

uint32_t movefromcreg( uint32_t addr ) {
    cpuid_opt_t opt;
    cpuid( 0x44, 0, addr, &opt );
    assert( opt.r_ecx == 0x42 );
    return opt.r_eax;
}

void freadrom( uint32_t addr, uint32_t *res ) {
    cpuid_opt_t opt;
    cpuid( 0x45, 0, addr, &opt );
    assert( opt.r_ecx == 0x42 );
    res[0] = opt.r_eax;
    res[1] = opt.r_edx;
}


void rdmsr_ring0( uint32_t msra , uint32_t *opt ) {
    uint32_t eax, edx;
    asm volatile ("rdmsr" : "=oa" (eax), "=d" (edx) : "c" (msra));
    opt[0] = eax;
    opt[1] = edx;
}

void usage( const char *reason ) {
    fprintf( stderr, "%s\n", reason );
    fprintf( stderr,
    "\tutool -h\n" );
    fprintf( stderr,
    "\tutool  [-d <readcount>] [-R <indexaddr> <crbus_addr>] [-w <crbus_addr> <value>] [-r <crbus_addr> [<count>]] [-f <fprom_addr> <count>] \n\n" );

    if ( !help_flag )
        exit( EXIT_FAILURE );

    fprintf( stderr,
    "\t\n"
    "\tProgram for manipulating internal state of Pentium II\n"
    "\twritten by Peter Bosch <public@pbx.sh>.\n"
    "\tThis program might crash or damage the system they are loaded onto\n"
    "\tand the author takes no responsibility for any damages resulting  \n"
    "\tfrom use of the software.\n"
    "\t\t-h                Print this message and exit\n"
    "\t\t\n"
    "\t\t-R                Perform indexed CRBUS read, write <crbus_addr>\n"
    "\t\t                  to <indexaddr> and read data from <indexaddr> + 1\n"
    "\t\t                  Optionally repeat from same address <readcount> the data read\n"
    "\t\t\n"
    "\t\t-r                Perform CRBUS read of <crbus_addr>\n"
    "\t\t                  Optionally do more reads up to <crbus_addr> + <count>\n"
    "\t\t                  Optionally repeat from same address <readcount> the data read\n"
    "\t\t\n"
    "\t\t-w                Perform CRBUS write to <crbus_addr>\n"
    "\t\t                  with <value>\n"
    "\t\t\n"
    "\t\t-f                Perform 64-bit FPROM data constant read from <fprom_addr>\n"
    "\t\t                  optionally do more reads up to <fprom_addr> + <count>\n"
    "\t\t\n");
}

void parse_args( int argc, char *const *argv, uint32_t *drange, uint32_t *idxaddr) {
    char opt;
    while ( (opt = getopt( argc, argv, "R:wrufd:" )) != -1 ) {
        switch( opt ) {
            case 'R':
                crbus_read_index_flag = 1;
                *idxaddr = strtoul(optarg, NULL, 0);
                break;
            case 'w':
                crbus_write_flag = 1;
                break;
            case 'r':
                crbus_read_flag = 1;
                break;
            case 'f':
                fprom_read_flag = 1;
                break;
            case 'd':
                *drange = strtoul(optarg, NULL, 0);
                break;
            case 'h':
                help_flag = 1;
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
    char brand[13];
    uint32_t l, r[2];
    unsigned int i,j;
    uint32_t reg;
    uint32_t val = 0, drange = 1, range = 1, addr, idxaddr;

    parse_args(argc, argv, &drange, &idxaddr);

    cpuid_leaf0(brand);
    if ( strcmp(brand, "peterbjornx ") != 0 ) {
        fprintf(stderr,"Invalid CPUID brand string: \"%s\"\n", brand);
        fprintf(stderr,
            "This program requires the \"cpuid_ext.dat\" microcode patch!\n");
        return 1;
    }

    if ((crbus_read_flag) || (fprom_read_flag) || (crbus_read_index_flag)) {
        if (optind == (argc - 1)) {
            reg = strtoul(argv[optind++], NULL, 0);
        } else if ((optind == (argc - 2)))  {
            reg = strtoul(argv[optind++], NULL, 0);
            range = strtoul(argv[optind++], NULL, 0);
        } else {
            usage("Wrong parameter count");
            exit(127);
        }
    } else if (crbus_write_flag) {
        if (optind != (argc - 2)) {
            usage("Wrong parameter count");
            exit(127);
        }
        reg = strtoul(argv[optind++], NULL, 0);
        val = strtoul(argv[optind++], NULL, 0);
        movetocreg(reg, val);
        printf("Wrote %x to CRBUS address %x\n", val, reg);
        return 0;
    } else {
        usage("Specify operation");
        exit(127);
    }

    for (i = 0;i < range; i++) {
        addr = reg + i;
        printf("\n%08X:", addr);
        if (crbus_read_index_flag) {
            movetocreg(idxaddr, addr);
        }

        for (j = 0;j < drange;j++) {
            if (fprom_read_flag) {
                     uint32_t res[2];
                freadrom(addr, &res[0]);
                     printf(" %08X", res[0]);
                val = res[1];
            } else if (crbus_read_flag) {
                val = movefromcreg(addr);
            } else if (crbus_read_index_flag) {
                val = movefromcreg(idxaddr + 1);
            } else {
                assert(!"cannot happen");
            }
            if ((drange > 1) && ((j % 8) == 0)) {
                printf("\n%04X:", j);
            }
            printf(" %08X", val);
        }
    }

    printf("\n");

    return 0;
}

/*
header_ver 0x00000001
update_rev 0x00000014
date_bcd   0x06101998
proc_sig   0x00000652
checksum   0x0E4A7195
loader_rev 0x00000001
proc_flags 0x00000001
data_size  0x00000000
total_size 0x00000000
key_seed   0x3BB08256
msram_file cpuid_ext.hex
write_creg 0x1B2 0x00000000 0x003E003B
write_creg 0x1B1 0x00000000 0x00000004
write_creg 0x1B8 0x00000000 0x2F043FB0
write_creg 0x11D 0x00000000 0x0001c401
write_creg 0x117 0x00000000 0x00000000
write_creg 0x116 0x00000000 0x00000000
write_creg 0x1B9 0x00000000 0x00000000
write_creg 0x1BA 0x00000000 0x00000000
write_creg 0x1BB 0x00000000 0x00000000
write_creg 0x1B3 0x00000000 0x00000000
write_creg 0x022 0xF7FFFFFF 0x08000000
write_creg 0x175 0x00000000 0x13371337
write_creg 0x1FF 0x00000000 0xB8D32993
write_creg 0x1FF 0x00000000 0x24778029
write_creg 0x1FF 0x00000000 0x939FA077
write_creg 0x1FF 0x00000000 0xCC11513B

UROM_3FAC                              U_JCC_NT_Z     (ALIAS_1ad     , addr_3FD2                                          , IA_11        , U2_08 , U3_1b )

UROM_3FAD addr_3FD2: ;  refd by:  UROM_3FAC 
UROM_3FAD       EOM_Fl2                   OP_0D8         (CONST_0       , EIP_30                                                            , U2_20         )

UROM_3FAE new_patch_start:
UROM_3FAE                              MOVE_DSZ32     (CONST         , CONST_0)
UROM_3FB0                              MOVE_DSZ32( CONST , CONST_0 )
UROM_3FB1                              MOVE_DSZ32( CONST , CONST_0 )
UROM_3FB2                              R34 = SUB_DSZ32 ( EAX , CONST_0 )
UROM_3FB4                              U_JCC_NT_NZ     ( R34 , cpuid_try_leaf42, IA_11, U3_1b )

UROM_3FB5                              EAX = MOVE_DSZ32( CONST, CONST_16_002 )

UROM_3FB6                              EBX = MOVE_DSZ32( CONST, CONST_16_065 )
UROM_3FB8                              EBX = SHL_DSZ32 ( EBX,   CONST_16_008 )
UROM_3FB9                              EBX = OR_DSZ32  ( EBX,   CONST_16_074 )
UROM_3FBA                              EBX = SHL_DSZ32 ( EBX,   CONST_16_008 )
UROM_3FBC                              EBX = OR_DSZ32  ( EBX,   CONST_16_065 )
UROM_3FBD                              EBX = SHL_DSZ32 ( EBX,   CONST_16_008 )
UROM_3FBE                              EBX = OR_DSZ32  ( EBX,   CONST_16_070 )

UROM_3FC0                              EDX = MOVE_DSZ32( CONST, CONST_16_06f )
UROM_3FC1                              EDX = SHL_DSZ32 ( EDX,   CONST_16_008 )
UROM_3FC2                                 EDX = OR_DSZ32  ( EDX,   CONST_16_06a )
UROM_3FC4                              EDX = SHL_DSZ32 ( EDX,   CONST_16_008 )
UROM_3FC5                              EDX = OR_DSZ32  ( EDX,   CONST_16_062 )
UROM_3FC6                              EDX = SHL_DSZ32 ( EDX,   CONST_16_008 )
UROM_3FC8                              EDX = OR_DSZ32  ( EDX,   CONST_16_072 )

UROM_3FC9                              ECX = MOVE_DSZ32( CONST, CONST_16_020 ) 
UROM_3FCA                              ECX = SHL_DSZ32 ( ECX,   CONST_16_008 )
UROM_3FCC                              ECX = OR_DSZ32  ( ECX,   CONST_16_078 )
UROM_3FCD                              ECX = SHL_DSZ32 ( ECX,   CONST_16_008 )
UROM_3FCE                              ECX = OR_DSZ32  ( ECX,   CONST_16_06e )
UROM_3FD0                              ECX = SHL_DSZ32 ( ECX,   CONST_16_008 )
UROM_3FD1                              ECX = OR_DSZ32  ( ECX,   CONST_16_072 )

UROM_3FD2                              U_JMP_NT        ( CONST, UROM_1E15, IA_11, U3_1b )

UROM_3FD4  cpuid_try_leaf42:
UROM_3FD4                              R34 = SUB_DSZ32 ( EAX,   CONST_16_042)
UROM_3FD5                              U_JCC_NT_NZ     ( R34,   cpuid_try_leaf43, IA_11, U3_1b )
UROM_3FD6                              MOVE_DSZ32      ( CONST         , CONST_0 )

UROM_3FD8  cpuid_leaf42:     
UROM_3FD8                              MOVETOCREG      (CONST_0e_188, EBX, U2_08)          
UROM_3FD9                              EAX = MOVEFROMCREG( CONST_0e_189, CONST, U2_20 )     
UROM_3FDA                              U_JMP_NT        ( CONST, cpuid_leaf4x_done, IA_11, U3_1b )  

UROM_3FDC  cpuid_try_leaf43:  
UROM_3FDC                              R34 = SUB_DSZ32 ( EAX,   CONST_16_043)               
UROM_3FDD                              U_JCC_NT_NZ     ( R34,   cpuid_try_leaf44, IA_11, U3_1b )
UROM_3FDE                              MOVE_DSZ32     (CONST         , CONST_0)

UROM_3FE0  cpuid_leaf43:
UROM_3FE0                              MOVETOCREG      ( ECX, EBX )
UROM_3FE1                              U_JMP_NT        ( CONST, cpuid_leaf4x_done, IA_11, U3_1b )  
UROM_3FE2                              MOVE_DSZ32     (CONST         , CONST_0)

UROM_3FE4  cpuid_try_leaf44:
UROM_3FE4                              R34 = SUB_DSZ32 ( EAX,   CONST_16_044 )
UROM_3FE5                              U_JCC_NT_NZ     ( R34,   cpuid_try_leaf45, IA_11, U3_1b )
UROM_3FE6                              MOVE_DSZ32     (CONST         , CONST_0)

UROM_3FE8  cpuid_leaf44:
UROM_3FE8                              EAX = MOVEFROMCREG( ECX, CONST_06_000 )
UROM_3FE9                              U_JMP_NT        ( CONST, cpuid_leaf4x_done, IA_11, U3_1b )
UROM_3FEA                              MOVE_DSZ32     (CONST         , CONST_0)

UROM_3FEC  cpuid_try_leaf45:
UROM_3FEC                              R34 = SUB_DSZ32 ( EAX,   CONST_16_045 )
UROM_3FED                              U_JCC_NT_NZ     ( R34,   cpuid_try_other, IA_11, U3_1b )
UROM_3FEE                              MOVE_DSZ32     (CONST         , CONST_0)

UROM_3FF0  cpuid_leaf45:
UROM_3FF0                              TMP0 = FREADROM ( CONST, ECX )
UROM_3FF1                              EAX  = MOVE_DSZ32 ( CONST, TMP0 )
UROM_3FF2                              EDX  = INTEXTRACT.HI32( TMP0, CONST_0 )
UROM_3FF4                              U_JMP_NT        ( CONST, cpuid_leaf4x_done, IA_11, U3_1b )
UROM_3FF5                              MOVE_DSZ32     (CONST         , CONST_0)
UROM_3FF6                              MOVE_DSZ32     (CONST         , CONST_0)
UROM_3FF8  cpuid_try_other:
UROM_3FF8                              EBX = MOVE_DSZ32 (CONST, CONST_0)
UROM_3FF9                              ECX = MOVE_DSZ32 (CONST, CONST_0)
UROM_3FFA                              U_JMP_NT        ( CONST, UROM_2F10, IA_11, U3_1b )
UROM_3FFC                              MOVE_DSZ32 (CONST, CONST_0)
UROM_3FFD  cpuid_leaf4x_done:
UROM_3FFD                              ECX = MOVE_DSZ32( CONST , CONST_16_042 )
UROM_3FFE                              U_JMP_NT        ( CONST, UROM_1E15, IA_11, U3_1b )
*/
