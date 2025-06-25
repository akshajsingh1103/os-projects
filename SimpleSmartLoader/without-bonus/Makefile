all:
	gcc  -m32 -no-pie -nostdlib -o sum sum.c
	gcc -m32 -o SimpleSmartLoader SimpleSmartLoader.c

run:
	./SimpleSmartLoader ./sum
clean:
	-@rm -f SimpleSmartLoader sum