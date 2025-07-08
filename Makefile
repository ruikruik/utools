
CFLAGS += $(CFLAGS-$@) -O2

CFLAGS-utool = -m32
CFLAGS-microload = -m32

all: microload utool ucode-66a ucode-652 ucode-634 msrom2scramble

ucode-66a:
	cat patches/ucode-exit.txt > patches/66a/utool-ucode.gen
	sed -e 's/UROM_CPUID_DONE/UROM_1CD1/g' -e 's/UROM_CPUID_OTHER/UROM_2B8A/g' < patches/utool-ucode.txt >> patches/66a/utool-ucode.gen
	python ../p6tools/simpleas.py patches/66a/utool-ucode.gen > patches/66a/utool-ucode_unscrambled.txt
	python ../p6tools/scramble.py patches/66a/utool-ucode_unscrambled.txt > patches/66a/utool-ucode.hex
	../patchtools_pub/patchtools -c -p patches/66a/utool-ucode-66a.dat -i patches/66a/utool-66a.txt

ucode-652:
	cat patches/ucode-exit.txt > patches/652/utool-ucode.gen
	sed -e 's/UROM_CPUID_DONE/UROM_1E15/g' -e 's/UROM_CPUID_OTHER/UROM_2F10/g' < patches/utool-ucode.txt >> patches/652/utool-ucode.gen
	python ../p6tools/simpleas.py patches/652/utool-ucode.gen > patches/652/utool-ucode_unscrambled.txt
	python ../p6tools/scramble.py patches/652/utool-ucode_unscrambled.txt > patches/652/utool-ucode.hex
	../patchtools_pub/patchtools -c -p patches/652/utool-ucode-652.dat -i patches/652/utool-652.txt

ucode-634:
	sed -e 's/UROM_CPUID_SKIP/UROM_1e21/g' <  patches/ucode-entry.txt > patches/634/utool-ucode.gen
	sed -e 's/UROM_CPUID_DONE/UROM_1401/g' -e 's/UROM_CPUID_OTHER/UROM_0A0A/g' < patches/utool-ucode.txt >> patches/634/utool-ucode.gen
	python ../p6tools/simpleas.py patches/634/utool-ucode.gen > patches/634/utool-ucode_unscrambled.txt
	python ../p6tools/scramble.py patches/634/utool-ucode_unscrambled.txt > patches/634/utool-ucode.hex
	../patchtools_pub/patchtools -c -p patches/634/utool-ucode-634.dat -i patches/634/utool-634.txt

microload : microload.c

utool : utool.c

msrom2scramble : msrom2scramble.c

clean :
	rm -f microload microload.exe utool utool.exe msrom2scramble msrom2scramble.exe patches/utool-ucode_unscrambled.txt
	rm -f patches/66a/utool-ucode.hex patches/66a/utool-ucode-66a.dat patches/66a/utool-ucode.gen patches/66a/utool-ucode_unscrambled.txt
	rm -f patches/652/utool-ucode.hex patches/652/utool-ucode-652.dat patches/652/utool-ucode.gen patches/652/utool-ucode_unscrambled.txt 
	rm -f patches/634/utool-ucode.hex patches/634/utool-ucode-634.dat patches/634/utool-ucode.gen patches/634/utool-ucode_unscrambled.txt 
