// microload.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <string.h>
#include <stdlib.h>
typedef unsigned short int     uint16_t;
typedef unsigned long int      uint32_t;
typedef struct {
	uint32_t r_eax;
	uint32_t r_ebx;
	uint32_t r_edx;
	uint32_t r_ecx;
} cpuid_opt_t;

volatile uint32_t rincr[4];

void call_ring0( void *targ ) {
	uint32_t idt_entry, r0vec, r0mod;
	r0mod = 0;
	__asm {
		call after_hnd
		pushad
		mov eax, targ
		call eax
		popad
		iretd
after_hnd:
		pop eax
		mov r0vec, eax
	}
	//r0vec = 0xc0001370;
	__asm {
		push eax
		sidt [esp-02h]
		pop ebx
		add ebx, 28
		cli
		mov ax, [ebx+02h]
		shl eax, 16
		mov ax, [ebx-04h]
		mov idt_entry, eax
		mov eax, r0vec
		mov [ebx-04h], ax
		shr eax, 16
		mov [ebx+02h], ax
		int 3
		mov eax, idt_entry
		mov [ebx-04h], ax
		shr eax, 16
		mov [ebx+02h], ax
		sti
	}
	fflush(stdout);
}

void rdmsr_ring0( void ) {
	uint32_t msra,lo,hi;
	msra = rincr[0];
	__asm {
		mov ecx, msra
		rdmsr
		mov lo, eax
		mov hi, edx
	}
	rincr[1] = lo;
	rincr[2] = hi;
}
void wrmsr_ring0( void ) {
	uint32_t msra, lo, hi;
	msra = rincr[0];
	lo = rincr[1];
	hi = rincr[2];
	__asm {
		mov ecx, msra
		mov eax, lo
		mov edx, hi
		wrmsr
	}
}

void rdmsr( uint32_t msra , uint32_t *opt ) {
	rincr[0] = msra;
	rincr[1] = 0xdeadbeef;
	rincr[2] = 0xdeadbeef;
	call_ring0( rdmsr_ring0 );
	opt[0] = rincr[1];
	opt[1] = rincr[2];
}

void wrmsr( uint32_t msra , uint32_t lo, uint32_t hi ) {
	rincr[0] = msra;
	rincr[1] = lo;
	rincr[2] = hi;
	call_ring0( wrmsr_ring0 );
}

void get_patchlvl_ring0() {
	uint32_t res;
	__asm {
		mov ecx, 08Bh
		xor eax, eax
		xor edx, edx
		wrmsr
		mov eax, 1
		cpuid
		mov ecx, 08Bh
		rdmsr
		mov res, edx
	}
	rincr[0] = res;
}

uint32_t get_patchlvl() {
	call_ring0(get_patchlvl_ring0);
	return rincr[0];
}

void cpuinfo( uint32_t leaf, cpuid_opt_t *opt ) {
	uint32_t a,b,c,d;
	__asm {
			mov eax, leaf
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

void cpuinfo() {
	uint32_t msrv[2];
	char brand[13];
	cpuid_opt_t opt;
	uint32_t leafs, l;
	cpuinfo( 0, &opt );
	leafs = opt.r_eax;
	brand[12] = 0;
	memcpy( brand, &opt.r_ebx, 12 );
	cpuinfo( 1, &opt );
	printf("CPU: \"%s\" Family %x Model %x Stepping %x\n", 
		brand,
		(opt.r_eax >> 8) & 0xf,
		(opt.r_eax >> 4) & 0xf,
		(opt.r_eax >> 0) & 0xf );
	printf("CPUID Level: %i, Features: EBX: %08X ECX: %08X EDX: %08X\n",
			leafs, opt.r_ebx, opt.r_ecx, opt.r_edx );
	printf("Microcode revision: %08X\n", get_patchlvl() );
	rdmsr( 0x175, msrv );
	printf("MSR 0x175: %08X %08X\n", msrv[0], msrv[1] );
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
	printf("\n--------------------- Before update -------------\n");	
	wrmsr(0x175,0xdeadc0de,0xcafecafe);
	cpuinfo();
	printf("\n----------------------  Do update  --------------\n");
	load_patch("a:\\newp6.dat");
	printf("\n---------------------- After update -------------\n");
	cpuinfo();
	return 0;
}
