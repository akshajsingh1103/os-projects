#include <stdio.h>
#include <stdlib.h>
#include "../loader/loader.h"  // adjust path to loader.h if needed

int main(int argc, char* argv[]) {
    if (argc < 2) {
        printf("Usage: %s <ELF file>\n", argv[0]);
        return 1;
    }

    void* elf_image = NULL;
    void* entry_segment = NULL;
    off_t filesize;
    uint32_t pfilesize;

    int result = load_and_run_elf(argv[1], &elf_image, &entry_segment, &filesize, &pfilesize);

    if (result < 0) {
        printf("Loading failed!\n");
        return 1;
    }

    loader_cleanup(entry_segment, elf_image, filesize, pfilesize);
    return 0;
}
