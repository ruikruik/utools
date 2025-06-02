Program for loading and interacting with custom microcode patches.
Intially written by Peter Bosch <public@pbx.sh> and ported to Linux
and extended by Rudolf Marek <r.marek@assembler.cz>.

# Disclaimer
Patchfiles produced by this program might crash or damage the system they are
loaded onto and the authors takes no responsibility for any damages resulting
from use of the software.

Only public resources and publically available hardware were used by the author
to produce this program.

# Compilation

The Makefile expects ``p6tools`` and ``patchtools_pub`` repositories to be placed a level up in the directories hiearchy and in case of ``patchtools_pub`` already build with make too.

To compile for Linux target, type "make".

To compile for DOS/DJGPP target, type ``CC=i586-pc-msdosdjgpp-gcc make``. Note that the DJGPP requires CWSDPMI runtime with ring 0 access,
therefore you will need to rename ``CWSDPR0.EXE`` to ``CWSDPMI.EXE``.

# Usage

Copy the tool to the target machine together with ``microload`` and ``utool`` and corresponding microcode update from ``patches/xxx/*.dat``.
Load the update using ``microload`` and then you can invoke ``utool``.

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

