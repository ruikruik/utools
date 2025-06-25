Programs for loading and interacting with custom microcode patches.
Intially written by Peter Bosch <public@pbx.sh> and ported to Linux
and extended by Rudolf Marek <r.marek@assembler.cz>.

# Disclaimer
Patchfiles produced by this program might crash or damage the system they are
loaded onto and the authors takes no responsibility for any damages resulting
from use of the software.

Only public resources and publically available hardware were used by the author
to produce this program.

# Compilation
Linux and DOS/DJGPP supported.

The Makefile expects ``p6tools`` and ``patchtools_pub`` repositories to be placed a level up in the directories hiearchy and in case of ``patchtools_pub`` already build with make too.

To compile for Linux target, type "make".

To compile for DOS/DJGPP target, type ``CC=i586-pc-msdosdjgpp-gcc make``. Note that the DJGPP requires CWSDPMI runtime with ring 0 access,
therefore you will need to rename ``CWSDPR0.EXE`` to ``CWSDPMI.EXE``.

# utool
Copy the ``utool`` to the target machine together with ``microload``  and corresponding microcode update from ``patches/xxx/*.dat``.
Load the update using ``microload`` and then you can invoke ``utool``.

## Supported Hardware
It works on Deschutes (cpuid 0x652) and Dixon (cpuid 0x66a). Support for new CPUs or features welcome.

## Usage
	utool -h
    utool  [-d <readcount>] [-R <indexaddr> <crbus_addr>] [-w <crbus_addr> <value>] [-r <crbus_addr> [<count>]] [-f <fprom_addr> <count>] 

    
    Program for manipulating internal state of Pentium II
    written by Peter Bosch <public@pbx.sh> and ported to Linux
    and extended by Rudolf Marek <r.marek@assembler.cz>

    This program might crash or damage the system they are loaded onto
    and the authors takes no responsibility for any damages resulting  
    from use of the software.
	-h                Print this message and exit
	
	-R                Perform indexed CRBUS read, write <crbus_addr>
	                  to <indexaddr> and read data from <indexaddr> + 1
	                  Optionally repeat from same address <readcount> the data read
	
	-r                Perform CRBUS read of <crbus_addr>
	                  Optionally do more reads up to <crbus_addr> + <count>
	                  Optionally repeat from same address <readcount> the data read
	
	-w                Perform CRBUS write to <crbus_addr>
	                  with <value>
	
	-f                Perform 64-bit FPROM data constant read from <fprom_addr>
	                  optionally do more reads up to <fprom_addr> + <count>

The ``msrom2scramble`` is meant to run on host system and transforms microcode ROM dumps to something which can be used by ``p6tools``.

# microload
All 2048 byte long microcodes can be loaded to any Intel CPUs. Mostly it is for Pentium Pro, Pentium, Pentium II, Pentium II and Pentium M systems.

## Usage
    microload -h
	microload  [-t <testfilename>] <filename> 

	
	Program for loading a microcode update into Intel CPU
	written by Peter Bosch <public@pbx.sh> and ported to Linux/DOS
	and extended by Rudolf Marek <r.marek@assembler.cz>

	This program might crash or damage the system they are loaded onto
	and the authors takes no responsibility for any damages resulting  
	from use of the software.
		-h                Print this message and exit
		
		-t                Perform side channel attack on microcode update.
		                  The <testfilename> specifies second update to be loaded
		                  in which each byte will be test-corrupted
		                  and results will be printed
		                  To make it work each update must be different revision
# msrom2scramble
 This program transforms the microcode ROM format read out from CRBUS MS_CR_ADDR / MS_CR_DATA register pair to the "scrambled" format of microcode used in the microcode updates.


