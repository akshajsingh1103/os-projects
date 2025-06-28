all:
	mkdir -p ../test
	gcc -m32 -no-pie -nostdlib -o ../test/fib ../test/fib.c
	gcc -m32 -fPIC -shared -o lib_simpleloader.so loader.c

clean:
	@rm -f ../test/fib lib_simpleloader.so
