all: microload utool ucode-66a ucode-652

utool-ucode:
	python ../p6tools/simpleas.py patches/utool-ucode.txt > patches/utool-ucode_unscrambled.txt
	python ../p6tools/scramble.py patches/utool-ucode_unscrambled.txt > patches/utool-ucode.hex

ucode-66a: utool-ucode
	../patchtools_pub/patchtools -c -p patches/66a/utool-ucode.dat -i patches/66a/utool-66a.txt

ucode-652: utool-ucode
	../patchtools_pub/patchtools -c -p patches/66a/utool-ucode.dat -i patches/652/utool-652.txt

microload :
	gcc -m32 -o microload microload.c

utool :
	gcc -m32 -o utool utool.c

clean :
	rm -f microload utool patches/utool-ucode_unscrambled.txt patches/utool-ucode.hex patches/66a/utool-ucode.dat patches/66a/utool-ucode.dat
