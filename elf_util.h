#include <elf.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

struct objfile {
    char *filename;
    char *address;
    size_t size;
};

char *get_section_name(Elf64_Ehdr *ehdr, Elf64_Shdr *shdr);

Elf64_Shdr *get_section(Elf64_Ehdr *ehdr, char *name);

void search_symbol(int n_objfiles, struct objfile objfiles[], char *symbol_name,
                   char **filename, char **address);
