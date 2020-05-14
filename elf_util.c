#include "elf_util.h"

char *get_section_name(Elf64_Ehdr *ehdr, Elf64_Shdr *shdr) {
    char *head = (char *)ehdr;
    Elf64_Shdr *sh = (Elf64_Shdr *)(head + ehdr->e_shoff +
                                    ehdr->e_shentsize * ehdr->e_shstrndx);
    return (char *)(head + sh->sh_offset + shdr->sh_name);
}

Elf64_Shdr *get_section(Elf64_Ehdr *ehdr, char *name) {
    char *head = (char *)ehdr;
    Elf64_Shdr *shdr = NULL;
    for (int i = 0; i < ehdr->e_shnum; i++) {
        Elf64_Shdr *s =
            (Elf64_Shdr *)(head + ehdr->e_shoff + ehdr->e_shentsize * i);
        if (strcmp(get_section_name(ehdr, s), name) == 0)
            shdr = s;
    }
    return shdr;
}

void search_symbol(int n_objfiles, struct objfile objfiles[], char *symbol_name,
                   char **filename, char **address) {
    for (int i = 0; i < n_objfiles; i++) {
        Elf64_Ehdr *ehdr;
        ehdr = (Elf64_Ehdr *)objfiles[i].address;
        if (!(ehdr->e_ident[0] == 0x7f && ehdr->e_ident[1] == 0x45 &&
              ehdr->e_ident[2] == 0x4c && ehdr->e_ident[3] == 0x46)) {
            printf("%s is not ELF file.\n", objfiles[i].filename);
            return;
        }

        Elf64_Shdr *shstr =
            (Elf64_Shdr *)(objfiles[i].address + ehdr->e_shoff +
                           ehdr->e_shentsize * ehdr->e_shstrndx);
        Elf64_Shdr *sh_symtab = NULL;
        Elf64_Shdr *sh_strtab = NULL;

        for (int j = 0; j < ehdr->e_shnum; j++) {
            Elf64_Shdr *shdr =
                (Elf64_Shdr *)(objfiles[i].address + ehdr->e_shoff +
                               ehdr->e_shentsize * j);
            char *name = (char *)(objfiles[i].address + shstr->sh_offset +
                                  shdr->sh_name);

            if (strcmp(name, ".symtab") == 0) {
                // printf("Found .symtab\n");
                sh_symtab = shdr;
            } else if (strcmp(name, ".strtab") == 0) {
                // printf("Found .strtab\n");
                sh_strtab = shdr;
            }
        }

        if (sh_symtab != NULL && sh_strtab != NULL) {
            int n_symbols = sh_symtab->sh_size / sh_symtab->sh_entsize;
            for (int j = 0; j < n_symbols; j++) {
                Elf64_Sym *sym =
                    (Elf64_Sym *)(objfiles[i].address + sh_symtab->sh_offset +
                                  sh_symtab->sh_entsize * j);
                if (sym->st_name != 0) {
                    char *sym_name = objfiles[i].address +
                                     sh_strtab->sh_offset + sym->st_name;
                    if (sym->st_shndx != SHN_ABS &&
                        sym->st_shndx != SHN_UNDEF &&
                        sym->st_shndx != SHN_COMMON &&
                        strcmp(sym_name, symbol_name) == 0) {
                        Elf64_Shdr *sh_sym_loc =
                            (Elf64_Shdr *)(objfiles[i].address + ehdr->e_shoff +
                                           ehdr->e_shentsize * sym->st_shndx);
                        // printf("symbol %s is located at filename = %s,
                        // section "
                        // "= %s, sh_sym_loc = %p, address "
                        // "= %p\n",
                        // symbol_name, objfiles[i].filename,
                        // get_section_name(ehdr, sh_sym_loc), sh_sym_loc,
                        // objfiles[i].address + sh_sym_loc->sh_offset +
                        // sym->st_value);
                        *filename = objfiles[i].filename;
                        *address =
                            (char *)(objfiles[i].address +
                                     sh_sym_loc->sh_offset + sym->st_value);
                    }
                }
            }
        }
    }
}
