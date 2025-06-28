all:
	gcc -m32 -o launch launch.c -L../loader -lsimpleloader -Wl,-rpath=../loader -I../loader

clean:
	@rm -f launch
