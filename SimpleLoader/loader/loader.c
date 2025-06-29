#include "loader.h"
#include <sys/stat.h>

void* g_entry_segment = NULL;
void* g_elf_mapped = NULL;
off_t g_elf_file_size = 0;
uint32_t g_loaded_segment_size = 0;
Elf32_Ehdr* g_elf_header = NULL;
Elf32_Phdr* g_program_headers = NULL;
int g_fd = -1;

// Forward declaration
int is_valid_elf(Elf32_Ehdr* header);

/*
 * Releases mapped memory regions and cleans up
 */
void loader_cleanup(void* entry_mem, void* elf_mem, off_t elf_size, uint32_t segment_size) {
    if (entry_mem){
        munmap(entry_mem, segment_size);
    }
    if (elf_mem){
        munmap(elf_mem, elf_size);
    }
}


/*
 * Load and run the ELF executable file
 */
void load_and_run_elf(char** argv) {
    // Open the ELF binary file
    int fd = open(argv[1], O_RDONLY);
    if (fd < 0) {
        perror("Failed to open ELF file");
        exit(EXIT_FAILURE);
    }

    // Get file size using fstat
    struct stat st;
    if (fstat(fd, &st) < 0) {
        perror("fstat failed");
        close(fd);
        exit(EXIT_FAILURE);
    }
    size_t filesize = st.st_size;

    // Map the entire ELF file into memory for reading headers
    void* mapped_elf = mmap(NULL, filesize, PROT_READ, MAP_PRIVATE, fd, 0);
    if (mapped_elf == MAP_FAILED) {
        perror("mmap failed");
        close(fd);
        exit(EXIT_FAILURE);
    }

    // Parse ELF header
    Elf32_Ehdr* hdr = (Elf32_Ehdr*)mapped_elf;
    if (!is_valid_elf(hdr)){
        fprintf(stderr, "Not a valid ELF file\n");
        close(fd);
        exit(EXIT_FAILURE);
    }

    // Locate the program header which contains the entry point
    Elf32_Phdr target_phdr;
    int found = 0;

    for (int i = 0; i < hdr->e_phnum; i++){
        Elf32_Phdr ph;
        off_t offset = hdr->e_phoff + i * hdr->e_phentsize;

        if (lseek(fd, offset, SEEK_SET) == -1 || read(fd, &ph, sizeof(ph)) != sizeof(ph)) {
            perror("Failed reading program header");
            close(fd);
            exit(EXIT_FAILURE);
        }

        if (ph.p_type == PT_LOAD &&
            hdr->e_entry >= ph.p_vaddr &&
            hdr->e_entry < (ph.p_vaddr + ph.p_memsz)) {
            target_phdr = ph;
            found = 1;
            break;
        }
    }

    if (!found) {
        fprintf(stderr, "Entry point segment not found.\n");
        close(fd);
        exit(EXIT_FAILURE);
    }

    // Allocate memory where the entry segment should be loaded
    void* segment_memory = mmap((void*)target_phdr.p_vaddr, target_phdr.p_memsz,
                                PROT_READ | PROT_WRITE | PROT_EXEC,
                                MAP_PRIVATE | MAP_FIXED | MAP_ANONYMOUS, -1, 0);

    if (segment_memory == MAP_FAILED) {
        perror("Segment mmap failed");
        close(fd);
        exit(EXIT_FAILURE);
    }

    // Load the segment from file into memory
    if (lseek(fd, target_phdr.p_offset, SEEK_SET) == -1 ||
        read(fd, segment_memory, target_phdr.p_filesz) != target_phdr.p_filesz) {
        perror("Failed to load segment data");
        close(fd);
        exit(EXIT_FAILURE);
    }

    // Jump to entry point
    int (*entry_func)() = (int(*)())hdr->e_entry;
    int retval = entry_func();

    printf("User _start return value = %d\n", retval);

    // Just for record (not used here)
    g_loaded_segment_size = target_phdr.p_memsz;


    close(fd);
}


// Verifies the ELF magic bytes to confirm valid ELF file
int is_valid_elf(Elf32_Ehdr* ehdr) {
    if (!ehdr) {
        fprintf(stderr, "Error: ELF header is NULL.\n");
        return 0;
    }

    if (ehdr->e_ident[EI_MAG0] != 0x7f ||
        ehdr->e_ident[EI_MAG1] != 'E'  ||
        ehdr->e_ident[EI_MAG2] != 'L'  ||
        ehdr->e_ident[EI_MAG3] != 'F') {
        fprintf(stderr, "Error: Invalid ELF magic bytes.\n");
        return 0;
    }

    return 1; // ELF file is valid
}

void loader_cleanup_all() {
    if (g_entry_segment) {
        munmap(g_entry_segment, g_loaded_segment_size);
    }
    if (g_elf_mapped) {
        munmap(g_elf_mapped, g_elf_file_size);
    }
    if (g_fd >= 0) {
        close(g_fd);
    }
}
