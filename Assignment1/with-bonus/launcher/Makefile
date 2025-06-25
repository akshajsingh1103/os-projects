#Compile the launch.c by linking it with the lib_simpleloader.so
launch: launch.c
	gcc -m32 -o launch launch.c ../loader/lib_simpleloader.so
#Provide the command for cleanup
clean:
	rm -f launch
.PHONY: clean