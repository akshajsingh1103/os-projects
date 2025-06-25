#invoke make inside following directories and in this order: loader, launch, fib
#move the lib_simpleloader.so and launch binaries inside bin directory
#Provide the command for cleanup
all:
	$(MAKE) -C loader
	$(MAKE) -C launcher
	$(MAKE) -C test	
	mkdir -p bin
	mv launcher/launch bin/
	cp test/fib bin/fib

run:
	cd bin && ./launch ./fib && rm ./fib

	

clean:
	$(MAKE) -C loader clean
	$(MAKE) -C launcher clean
	$(MAKE) -C test	clean
	rm -rf bin
