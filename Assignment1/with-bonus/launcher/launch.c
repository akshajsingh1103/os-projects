#include "../loader/loader.h"
#include "../loader/loader.c"
int main(int argc, char** argv) 
{
  if(argc != 2) {
    printf("Usage: %s <ELF Executable> \n",argv[0]);
    exit(1);
  }
  // 1. carry out necessary checks on the input ELF file -> this is done inside an auxilliary function in loader.c called ElfChecker()

  // 2. passing it to the loader for carrying out the loading/execution
  load_and_run_elf(argv);
  // 3. invoke the cleanup routine inside the loader
  loader_cleanup(entry_segment,elfStorage,fileSize,p_filesize_hello);
  /*cleanup routine is invoked within the load_and_run_elf() function itself*/  
  return 0;
}