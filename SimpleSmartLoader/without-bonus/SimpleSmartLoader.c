#include "SimpleSmartLoader.h"
#define MAP_ANONYMOUS	0x20

off_t fileSize;
int fd;
Elf32_Ehdr *ehdr;
int globalNPAGEcount=0;
int numSegfaults = -1;
size_t fragmentation = 0;

struct cleanup{
    void* address;
    int numpages;
};
struct cleanup arr[10];

void loader_cleanup()
{
    for(int i = 0; i <= numSegfaults; i++){
        munmap(arr[i].address,arr[i].numpages*PAGESZ);
    }
    free(ehdr);
}

Elf32_Phdr returnPtype(void* fault_address)
{
    Elf32_Phdr temp;
    off_t programheadertable = ehdr->e_phoff;
    for(int i = 0; i < ehdr->e_phnum; i++)
    {
        if(lseek(fd,programheadertable + i*ehdr->e_phentsize,SEEK_SET)== -1){
            perror("lseek fail");
            exit(EXIT_FAILURE);
        }
        if(read(fd,&temp,sizeof(Elf32_Phdr)) != sizeof(Elf32_Phdr)){
            perror("read fail");
            exit(EXIT_FAILURE);
        }
        if(fault_address >= (void*)temp.p_vaddr && fault_address < (void*)(temp.p_vaddr + temp.p_memsz)){
            return temp;
        }
    }
}



void sigsegv_handler(int signum, siginfo_t* info, void* context){
    // fault_address = (void*)malloc(sizeof(Elf32_Phdr));
    numSegfaults++;
    void* fault_address = info->si_addr;

    // int a = ((Elf32_Phdr*)fault_address)->p_type;
    // printf("ptype is %i\n",a);

    Elf32_Phdr temp= returnPtype(fault_address);
    int npages;
    off_t remain = temp.p_memsz%PAGESZ;
    if(remain== 0){
        npages= temp.p_memsz/PAGESZ;
    }
    else {
        npages= (temp.p_memsz/PAGESZ) + 1;
        fragmentation += PAGESZ-remain;
    }
    globalNPAGEcount+= npages;
    void* allocated_memory = mmap((void*)fault_address,
        npages*PAGESZ,
        PROT_READ | PROT_WRITE | PROT_EXEC,
        MAP_ANONYMOUS |MAP_PRIVATE,
        -1, 0);
    arr[numSegfaults].address = (void*)fault_address;
    arr[numSegfaults].numpages = npages;
    if(lseek(fd,temp.p_offset,SEEK_SET) == -1){//exiting if lseek failed
      close(fd);
      exit(EXIT_FAILURE);
    }
    read(fd,allocated_memory,temp.p_filesz);//exiting if size mismatch
   
    printf("fault addr is %p\n", fault_address);
   //memcpy(allocated_memory,ehdr + temp.p_offset,entry_phdr->p_memsz);
    fprintf(stderr, "Segmentation Fault (SIGSEGV) received. Exiting.\n");
    return;
}

void load_and_run_elf(char** exe)
{
    struct sigaction sa;
    sa.sa_sigaction = sigsegv_handler;
    sa.sa_flags = SA_SIGINFO;  
    if(sigaction(SIGSEGV,&sa,NULL) == -1){
        perror("sigaction");
        return ;
    }

    fd = open(exe[1], O_RDONLY);
    struct stat fileStatus;
    if(fstat(fd, &fileStatus) == -1){//checks status of file
        close(fd);
        exit(EXIT_FAILURE);
    }
    fileSize = fileStatus.st_size;
    ehdr=(Elf32_Ehdr*)malloc(sizeof(Elf32_Ehdr));
    read(fd,ehdr,sizeof(Elf32_Ehdr));
    // for(int i = 0; i < ehdr->e_phnum; i++){
    //     if(lseek(fd,(ehdr->e_phoff) + (i*ehdr->e_phentsize), SEEK_SET) == -1){//exiting program if lseek fails
    //         close(fd);
    //         exit(EXIT_FAILURE);
    //     }
    //     if(read(fd,&phdr,sizeof(Elf32_Phdr)) != sizeof(Elf32_Phdr)){//exiting program if size mismatch
    //         close(fd);
    //         exit(EXIT_FAILURE);
    //     }
    //     if(phdr.p_type == 1 && ehdr->e_entry >= phdr.p_vaddr && ehdr->e_entry < (phdr.p_vaddr+phdr.p_memsz )){//found entrypoint 
    // printf("found\n");
    //         entry_phdr = &phdr;
    //         break;
    //     }
    // }
    // int* ptr = (int*)0x11111111;
    // *ptr = 5;
    int(*_start)() = (int(*)())(ehdr->e_entry);
    int result = _start();
    printf("----------------------------------------------------\n");
    printf("User _start return value = %d\n",result);
    printf("Number of segfaults is %i\n", ++numSegfaults);
    printf("No of pages used is %i\n", globalNPAGEcount);
    printf("Fragmenation is %zu bytes \n",fragmentation);

}


int main(int argc, char** argv) 
{
    if(argc != 2) {
        printf("Usage: %s <ELF Executable> \n",argv[0]);
        exit(1);
    }
    load_and_run_elf(argv);
    return 0;
}