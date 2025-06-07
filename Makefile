
CFLAGS += $(CFLAGS-$@)

CFLAGS-utool = -m32
CFLAGS-microload = -m32

all: microload utool ucode-66a ucode-652 msrom2scramble

utool-ucode:
	python ../p6tools/simpleas.py patches/utool-ucode.txt > patches/utool-ucode_unscrambled.txt
	python ../p6tools/scramble.py patches/utool-ucode_unscrambled.txt > patches/utool-ucode.hex

ucode-66a: utool-ucode
	../patchtools_pub/patchtools -c -p patches/66a/utool-ucode-66a.dat -i patches/66a/utool-66a.txt

ucode-652: utool-ucode
	../patchtools_pub/patchtools -c -p patches/652/utool-ucode-652.dat -i patches/652/utool-652.txt

microload : microload.c

utool : utool.c

msrom2scramble : msrom2scramble.c

clean :
	rm -f microload microload.exe utool utool.exe msrom2scramble msrom2scramble.exe patches/utool-ucode_unscrambled.txt patches/utool-ucode.hex patches/66a/utool-ucode-66a.dat patches/652/utool-ucode-652.dat
