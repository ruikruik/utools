all: microload utool

microload :
	gcc -m32 -o microload microload.c

utool :
	gcc -m32 -o utool utool.c

clean :
	rm microload utool
