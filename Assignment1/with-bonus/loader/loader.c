#include "loader.h"
#include <sys/stat.h>
void* entry_segment;
void* elfStorage;
off_t fileSize;
uint32_t p_filesize_hello;
Elf32_Ehdr *ehdr;
Elf32_Phdr *phdr;
int fd;
int elfChecker(Elf32_Ehdr* header);

/*
 * release memory and other cleanups
 */
void loader_cleanup(void* entry_segment, void* elfimage, off_t filesize, uint32_t pfilesize) {
  //deallocating entry segment
  munmap(entry_segment,pfilesize);
  munmap(elfimage,filesize);
}

/*
 * Load and run the ELF executable file
 */
void load_and_run_elf(char** exe) {
  fd = open(exe[1], O_RDONLY);
  if(fd == -1){
    perror("error opening\n");
    exit(EXIT_FAILURE);
  }
  //gnu c manual instructs to use off_t for file size in bytes so that's what I used
  struct stat fileStatus;
  if(fstat(fd, &fileStatus) == -1){//checks status of file
    close(fd);
    exit(EXIT_FAILURE);
  }
  fileSize = fileStatus.st_size;

  // 1. Load entire binary content into the memory from the ELF file.
  elfStorage = mmap(NULL,fileSize,PROT_READ,MAP_PRIVATE,fd,0);
  if(elfStorage == MAP_FAILED){//checking if the mmap was successful
    close(fd);
    exit(EXIT_FAILURE);
  } 

  ehdr =  (Elf32_Ehdr*)elfStorage;//type casted the void pointer to Elf32 Ehdr
  int x = elfChecker(ehdr);//checking for a valid elf file
  if(x != 1) exit(EXIT_FAILURE);


  // 2. Iterate through the PHDR table and find the section of PT_LOAD 
  // type that contains the address of the entrypoint method in fib.c
  Elf32_Phdr entry_phdr;
  for(int i = 0; i < ehdr->e_phnum; i++){
    Elf32_Phdr phdr;
    if(lseek(fd,(ehdr->e_phoff) + (i*ehdr->e_phentsize), SEEK_SET) == -1){//exiting program if lseek fails
      close(fd);
      exit(EXIT_FAILURE);
    }
    if(read(fd,&phdr,sizeof(Elf32_Phdr)) != sizeof(Elf32_Phdr)){//exiting program if size mismatch
      close(fd);
      exit(EXIT_FAILURE);
    }
    if(phdr.p_type == 1 && ehdr->e_entry >= phdr.p_vaddr && ehdr->e_entry < (phdr.p_vaddr+phdr.p_memsz )){//found entrypoint 
      entry_phdr = phdr;
      break;
    }
  }  
  
  // 3. Allocate memory of the size "p_memsz" using mmap function 
  //    and then copy the segment content
  entry_segment = mmap(
        (void*)entry_phdr.p_vaddr,
        entry_phdr.p_memsz,
        PROT_READ | PROT_WRITE | PROT_EXEC,
        MAP_PRIVATE | MAP_FIXED | MAP_ANONYMOUS,
        -1, 0);
    //maapping memory in segment
    
    if (entry_segment == MAP_FAILED){//checking if map was successful 
        perror("mmap");
        close(fd);
        exit(EXIT_FAILURE);
    }

    //reading segment data into memory
    if(lseek(fd,entry_phdr.p_offset,SEEK_SET) == -1){//exiting if lseek failed
      close(fd);
      exit(EXIT_FAILURE);
    }
    if(read(fd,entry_segment,entry_phdr.p_filesz) != entry_phdr.p_filesz){//exiting if size mismatch
      close(fd);
      exit(EXIT_FAILURE);
    }
    
  //4. Navigate to the entrypoint address into the segment loaded in the memory in above step
  //5. Typecast the address to that of function pointer matching "_start" method in fib.c.
  int(*_start)() = (int(*)())ehdr->e_entry;
  // 6. Call the "_start" method and print the value returned from the "_start"
  int result = _start();
  printf("User _start return value = %d\n",result);
  p_filesize_hello = entry_phdr.p_memsz;
  
}

//checks if the elf file is in correct format
int elfChecker(Elf32_Ehdr* header){
 if(header == NULL) {
   printf("NULL pointer to elf header\n");
   return 0;
 }
 if(header->e_ident[EI_MAG0] != ELFMAG0){//checks if the value of the first byte of the ELF identification array(e_ident) is equal to 0x7f
   printf("First Byte Incorrect\n");
   return 0;
 }
 if(header->e_ident[EI_MAG1] != ELFMAG1){//second byte
   printf("Second Byte Incorrect\n");
   return 0;
 }
 if(header->e_ident[EI_MAG2] != ELFMAG2){//second byte
   printf("Second Byte Incorrect\n");
   return 0;
 }
 if(header->e_ident[EI_MAG3] != ELFMAG3) {//third byte
   printf("Third Byte Incorrect\n");
   return 0;
 }
 return 1;
}