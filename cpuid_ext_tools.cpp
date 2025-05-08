// cpuid_ext_tools.cpp : Defines the entry point for the console application.
// written by @peterbjornx https://twitter.com/peterbjornx

#include "stdafx.h"
#include <string.h>
#include <stdlib.h>
#include <assert.h>
typedef unsigned short int     uint16_t;
typedef unsigned long int      uint32_t;
typedef struct {
	uint32_t r_eax;
	uint32_t r_ebx;
	uint32_t r_edx;
	uint32_t r_ecx;
} cpuid_opt_t;

void cpuid( uint32_t leaf, uint32_t a_ebx, uint32_t a_ecx, cpuid_opt_t *opt ) {
	uint32_t a,b,c,d;
	__asm {
			mov eax, leaf
			mov ebx, a_ebx
			mov ecx, a_ecx
			cpuid
			mov a, eax
			mov b, ebx
			mov c, ecx
			mov d, edx
	}
	opt->r_eax = a;
	opt->r_ebx = b;
	opt->r_ecx = c;
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

void rdmsr_ring0( uint32_t msra, uint32_t *out) {
	uint32_t lo, hi;
	__asm {
		mov ecx, msra
		rdmsr
		mov lo, eax
		mov hi, edx
	}
	out[0] = lo;
	out[1] = hi;
}

int main(int argc, char* argv[])
{
	char brand[13];
	uint32_t l, r[2];
	cpuid_leaf0(brand);
	if ( strcmp(brand, "peterbjornx ") != 0 ) {
		fprintf(stderr,"Invalid CPUID brand string: \"%s\"\n", brand);
		fprintf(stderr,
			"This program requires the \"cpuid_ext.dat\" microcode patch!\n");
	}
	l = 0x4242C0DE;
	printf("Read CREG 0x175: %08X\n", movefromcreg(0x175));
	printf("Write CREG 0x175: %08X\n", l);
	movetocreg(0x175, l);
	printf("Writing 0 to P6_CR_CPL followed by rdmsr\n");
	movetocreg(0x101,0);
	//Had printf here, but Win98 does not like calling libc in ring 0
	rdmsr_ring0(0x175,r);
	movetocreg(0x101,3);
	printf("Written 3 back to P6_CR_CPL\n");
	printf("MSR 0x175 = 0x%08X%08X\n", r[1], r[0] );
	printf("Read CREG 0x175: %08X\n", movefromcreg(0x175));
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

UROM_3FAC	                          U_JCC_NT_Z     (ALIAS_1ad     , addr_3FD2                                          , IA_11        , U2_08 , U3_1b )

UROM_3FAD addr_3FD2: ;  refd by:  UROM_3FAC 
UROM_3FAD       EOM_Fl2                   OP_0D8         (CONST_0       , EIP_30                                                            , U2_20         )

UROM_3FAE new_patch_start:
UROM_3FAE	                          MOVE_DSZ32     (CONST         , CONST_0)
UROM_3FB0	                          MOVE_DSZ32( CONST , CONST_0 )
UROM_3FB1	                          MOVE_DSZ32( CONST , CONST_0 )
UROM_3FB2	                          R34 = SUB_DSZ32 ( EAX , CONST_0 )
UROM_3FB4	                          U_JCC_NT_NZ     ( R34 , cpuid_try_leaf42, IA_11, U3_1b )

UROM_3FB5	                          EAX = MOVE_DSZ32( CONST, CONST_16_002 )

UROM_3FB6	                          EBX = MOVE_DSZ32( CONST, CONST_16_065 )
UROM_3FB8	                          EBX = SHL_DSZ32 ( EBX,   CONST_16_008 )
UROM_3FB9	                          EBX = OR_DSZ32  ( EBX,   CONST_16_074 )
UROM_3FBA	                          EBX = SHL_DSZ32 ( EBX,   CONST_16_008 )
UROM_3FBC	                          EBX = OR_DSZ32  ( EBX,   CONST_16_065 )
UROM_3FBD	                          EBX = SHL_DSZ32 ( EBX,   CONST_16_008 )
UROM_3FBE	                          EBX = OR_DSZ32  ( EBX,   CONST_16_070 )

UROM_3FC0	                          EDX = MOVE_DSZ32( CONST, CONST_16_06f )
UROM_3FC1	                          EDX = SHL_DSZ32 ( EDX,   CONST_16_008 )
UROM_3FC2                                 EDX = OR_DSZ32  ( EDX,   CONST_16_06a )
UROM_3FC4	                          EDX = SHL_DSZ32 ( EDX,   CONST_16_008 )
UROM_3FC5	                          EDX = OR_DSZ32  ( EDX,   CONST_16_062 )
UROM_3FC6	                          EDX = SHL_DSZ32 ( EDX,   CONST_16_008 )
UROM_3FC8	                          EDX = OR_DSZ32  ( EDX,   CONST_16_072 )

UROM_3FC9	                          ECX = MOVE_DSZ32( CONST, CONST_16_020 ) 
UROM_3FCA	                          ECX = SHL_DSZ32 ( ECX,   CONST_16_008 )
UROM_3FCC	                          ECX = OR_DSZ32  ( ECX,   CONST_16_078 )
UROM_3FCD	                          ECX = SHL_DSZ32 ( ECX,   CONST_16_008 )
UROM_3FCE	                          ECX = OR_DSZ32  ( ECX,   CONST_16_06e )
UROM_3FD0	                          ECX = SHL_DSZ32 ( ECX,   CONST_16_008 )
UROM_3FD1	                          ECX = OR_DSZ32  ( ECX,   CONST_16_072 )

UROM_3FD2	                          U_JMP_NT        ( CONST, UROM_1E15, IA_11, U3_1b )

UROM_3FD4  cpuid_try_leaf42:
UROM_3FD4	                          R34 = SUB_DSZ32 ( EAX,   CONST_16_042)
UROM_3FD5	                          U_JCC_NT_NZ     ( R34,   cpuid_try_leaf43, IA_11, U3_1b )
UROM_3FD6	                          MOVE_DSZ32      ( CONST         , CONST_0 )

UROM_3FD8  cpuid_leaf42:	 
UROM_3FD8	                          MOVETOCREG      (CONST_0e_188, EBX, U2_08)          
UROM_3FD9	                          EAX = MOVEFROMCREG( CONST_0e_189, CONST, U2_20 )     
UROM_3FDA	                          U_JMP_NT        ( CONST, cpuid_leaf4x_done, IA_11, U3_1b )  

UROM_3FDC  cpuid_try_leaf43:  
UROM_3FDC	                          R34 = SUB_DSZ32 ( EAX,   CONST_16_043)	           
UROM_3FDD	                          U_JCC_NT_NZ     ( R34,   cpuid_try_leaf44, IA_11, U3_1b )
UROM_3FDE	                          MOVE_DSZ32     (CONST         , CONST_0)

UROM_3FE0  cpuid_leaf43:
UROM_3FE0	                          MOVETOCREG      ( ECX, EBX )
UROM_3FE1	                          U_JMP_NT        ( CONST, cpuid_leaf4x_done, IA_11, U3_1b )  
UROM_3FE2	                          MOVE_DSZ32     (CONST         , CONST_0)

UROM_3FE4  cpuid_try_leaf44:
UROM_3FE4	                          R34 = SUB_DSZ32 ( EAX,   CONST_16_044 )
UROM_3FE5	                          U_JCC_NT_NZ     ( R34,   cpuid_try_leaf45, IA_11, U3_1b )
UROM_3FE6	                          MOVE_DSZ32     (CONST         , CONST_0)

UROM_3FE8  cpuid_leaf44:
UROM_3FE8	                          EAX = MOVEFROMCREG( ECX, CONST_06_000 )
UROM_3FE9	                          U_JMP_NT        ( CONST, cpuid_leaf4x_done, IA_11, U3_1b )
UROM_3FEA	                          MOVE_DSZ32     (CONST         , CONST_0)

UROM_3FEC  cpuid_try_leaf45:
UROM_3FEC	                          R34 = SUB_DSZ32 ( EAX,   CONST_16_045 )
UROM_3FED	                          U_JCC_NT_NZ     ( R34,   cpuid_try_other, IA_11, U3_1b )
UROM_3FEE	                          MOVE_DSZ32     (CONST         , CONST_0)

UROM_3FF0  cpuid_leaf45:
UROM_3FF0	                          TMP0 = FREADROM ( CONST, ECX )
UROM_3FF1	                          EAX  = MOVE_DSZ32 ( CONST, TMP0 )
UROM_3FF2	                          EDX  = INTEXTRACT.HI32( TMP0, CONST_0 )
UROM_3FF4	                          U_JMP_NT        ( CONST, cpuid_leaf4x_done, IA_11, U3_1b )
UROM_3FF5	                          MOVE_DSZ32     (CONST         , CONST_0)
UROM_3FF6	                          MOVE_DSZ32     (CONST         , CONST_0)
UROM_3FF8  cpuid_try_other:
UROM_3FF8	                          EBX = MOVE_DSZ32 (CONST, CONST_0)
UROM_3FF9	                          ECX = MOVE_DSZ32 (CONST, CONST_0)
UROM_3FFA	                          U_JMP_NT        ( CONST, UROM_2F10, IA_11, U3_1b )
UROM_3FFC	                          MOVE_DSZ32 (CONST, CONST_0)
UROM_3FFD  cpuid_leaf4x_done:
UROM_3FFD	                          ECX = MOVE_DSZ32( CONST , CONST_16_042 )
UROM_3FFE	                          U_JMP_NT        ( CONST, UROM_1E15, IA_11, U3_1b )
*/