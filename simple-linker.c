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

char *get_section_name(Elf64_Ehdr *ehdr, Elf64_Shdr *shdr){
    Elf64_Shdr *sh = (Elf64_Shdr *)(ehdr + ehdr->e_shoff + ehdr->e_shentsize * ehdr->e_shstrndx);
    return (char *)(ehdr + sh->sh_offset + shdr->sh_name);
}

Elf64_Shdr *get_section(Elf64_Ehdr *ehdr, char *name){
    Elf64_Shdr *shdr = NULL;
    for(int i=0;i<ehdr->e_shnum;i++){
        Elf64_Shdr *s = (Elf64_Shdr *)(ehdr + ehdr->e_shoff + ehdr->e_shentsize * i);
        if(strcmp(get_section_name(ehdr, s), name) == 0)
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
                        printf("symbol %s is located at\n", symbol_name);
                        printf("filename = %s, address = %p\n",
                               objfiles[i].filename,
                               objfiles[i].address + sh_sym_loc->sh_offset +
                                   sym->st_value);
                        *filename = objfiles[i].filename;
                        *address = objfiles[i].address + sh_sym_loc->sh_offset +
                                   sym->st_value;
                    }
                }
            }
        }
    }
}

int analyse_one_objfile(char *objfilename) {
    int fd;
    struct stat sb;
    char *head;

    fd = open(objfilename, O_RDONLY);
    fstat(fd, &sb);
    printf("size of %s = %ld\n", objfilename, sb.st_size);
    head = mmap(NULL, sb.st_size, PROT_READ, MAP_SHARED, fd, 0);

    Elf64_Ehdr *ehdr;
    ehdr = (Elf64_Ehdr *)head;
    if (!(ehdr->e_ident[0] == 0x7f && ehdr->e_ident[1] == 0x45 &&
          ehdr->e_ident[2] == 0x4c && ehdr->e_ident[3] == 0x46)) {
        printf("%s is not ELF file.\n", objfilename);
        return 0;
    }

    /* #define EM_X86_64	62 */
    /* AMD x86-64 architecture */
    printf("e_machine = %d\n", ehdr->e_machine);
    printf("ehdr->e_shoff = %ld\n", ehdr->e_shoff);

    Elf64_Shdr *shstr = (Elf64_Shdr *)(head + ehdr->e_shoff +
                                       ehdr->e_shentsize * ehdr->e_shstrndx);
    Elf64_Shdr *sh_symtab = NULL;
    Elf64_Shdr *sh_strtab = NULL;
    Elf64_Shdr *sh_rela = NULL;

    for (int i = 0; i < ehdr->e_shnum; i++) {
        Elf64_Shdr *shdr =
            (Elf64_Shdr *)(head + ehdr->e_shoff + ehdr->e_shentsize * i);
        char *name = (char *)(head + shstr->sh_offset + shdr->sh_name);
        printf("%s\n", name);

        if (strcmp(name, ".symtab") == 0) {
            printf("Found .symtab\n");
            sh_symtab = shdr;
        } else if (strcmp(name, ".strtab") == 0) {
            printf("Found .strtab\n");
            sh_strtab = shdr;
        } else if (strcmp(name, ".rela.text") == 0) {
            printf("Found .rela.text\n");
            sh_rela = shdr;
        }
    }

    if (sh_strtab != NULL) {
        int n_strs = 0;
        char **strs = malloc(sizeof(char *) * sh_strtab->sh_size);
        for (int64_t i = (int64_t)head + sh_strtab->sh_offset;
             i < (int64_t)head + sh_strtab->sh_offset + sh_strtab->sh_size - 1;
             i++) {
            // printf("i = %ld\n", i);
            // printf("*(char *)i = %c\n", *(char *)i);
            if (*(char *)i == '\0') {
                n_strs++;
                strs[n_strs] = (char *)i + 1;
            }
        }
        for (int i = 1; i <= n_strs; i++) {
            printf("Print %dth str: %s\n", i, strs[i]);
        }
    }

    if (sh_symtab != NULL) {
        int n_symbols = sh_symtab->sh_size / sh_symtab->sh_entsize;
        for (int i = 0; i < n_symbols; i++) {
            Elf64_Sym *sym = (Elf64_Sym *)(head + sh_symtab->sh_offset +
                                           sh_symtab->sh_entsize * i);
            if (sym->st_name != 0) {
                printf("The name of #%d symbol = %s\n", i,
                       head + sh_strtab->sh_offset + sym->st_name);
                if (sym->st_shndx != SHN_ABS && sym->st_shndx != SHN_UNDEF &&
                    sym->st_shndx != SHN_COMMON) {
                    Elf64_Shdr *sh_sym_loc =
                        (Elf64_Shdr *)(head + ehdr->e_shoff +
                                       ehdr->e_shentsize * sym->st_shndx);
                    printf("address = %p\n",
                           head + sh_sym_loc->sh_offset + sym->st_value);
                }
            }
        }
    }

    if (sh_rela != NULL) {
        int n_rela = sh_rela->sh_size / sh_rela->sh_entsize;
        for (int i = 0; i < n_rela; i++) {
            Elf64_Rela *rela = (Elf64_Rela *)(head + sh_rela->sh_offset +
                                              sh_rela->sh_entsize * i);
            printf("#%d rela entry: r_offset = %ld, ELF64_R_SYM(r_info) = %lx "
                   "addend = %lx\n",
                   i, rela->r_offset, ELF64_R_SYM(rela->r_info),
                   rela->r_addend);
        }
    }
    return 0;
}

int main(int argc, char *argv[]) {
    // for (int i = 1; i < argc; i++) {
    //     printf("========== %s ==========\n", argv[i]);
    //     analyse_one_objfile(argv[i]);
    // }

    struct objfile *objfiles = malloc(sizeof(struct objfile) * (argc - 1));
    int n_objfiles = argc-1;
    for (int i = 1; i < argc; i++) {
        objfiles[i - 1].filename = argv[i];

        struct stat sb;
        int fd = open(objfiles[i - 1].filename, O_RDONLY);
        fstat(fd, &sb);
        objfiles[i - 1].address =
            mmap(NULL, sb.st_size, PROT_READ, MAP_SHARED, fd, 0);
        objfiles[i-1].size = sb.st_size;
    }

    printf("========== search_symbol ==========\n");
    while (0) {
        char *filename;
        char *address;
        char buf[100];
        printf("the name of the symbol: ");
        fgets(buf, 100, stdin);
        char *pos;
        if ((pos = strchr(buf, '\n')) != NULL)
            *pos = '\0';
        else
            return 0;
        search_symbol(argc - 1, objfiles, buf, &filename, &address);
    }

    char *buffer;
    buffer = malloc(sizeof(char) * 64 * 1024);
    buffer =
        (char *)(((uint64_t)buffer + ((1 << 12) - 1)) & (~((1 << 12) - 1)));
    printf("buffer = %p\n", buffer);
    char *p;
    p = buffer;

    Elf64_Ehdr *ehdr;
    ehdr = (Elf64_Ehdr *)p;
    ehdr->e_ident[EI_MAG0] = ELFMAG0;
    ehdr->e_ident[EI_MAG1] = ELFMAG1;
    ehdr->e_ident[EI_MAG2] = ELFMAG2;
    ehdr->e_ident[EI_MAG3] = ELFMAG3;
    ehdr->e_ident[EI_CLASS] = ELFCLASS64;
    ehdr->e_ident[EI_DATA] = ELFDATA2LSB;
    ehdr->e_ident[EI_VERSION] = EV_CURRENT;
    ehdr->e_ident[EI_OSABI] = ELFOSABI_LINUX;
    ehdr->e_ident[EI_ABIVERSION] = 0;
    ehdr->e_ident[EI_PAD] = 0;

    ehdr->e_type = ET_EXEC;
    ehdr->e_machine = EM_X86_64;
    ehdr->e_version = EV_CURRENT;

    ehdr->e_entry = 0;
    ehdr->e_phoff = sizeof(Elf64_Ehdr);
    ehdr->e_shoff = 0;
    ehdr->e_flags = 0;
    ehdr->e_ehsize = sizeof(Elf64_Ehdr);
    ehdr->e_phentsize = sizeof(Elf64_Phdr);
    ehdr->e_phnum = 1;
    ehdr->e_shentsize = 0;
    ehdr->e_shnum = 0;
    ehdr->e_shstrndx = 0;

    Elf64_Phdr *phdr = (Elf64_Phdr *)((char *)ehdr + ehdr->e_phoff);
    phdr->p_type = PT_LOAD;
    phdr->p_offset = 0;
    phdr->p_vaddr = (Elf64_Addr)p;
    phdr->p_paddr = (Elf64_Addr)p;
    // phdr->p_vaddr = p;
    // phdr->p_paddr = p;
    printf("sizeof(char *) = %ld, sizeof(Elf64_Ehdr) = %ld, sizeof(Elf64_Phdr) = %ld\n", sizeof(char *), sizeof(Elf64_Ehdr), sizeof(Elf64_Phdr));
    printf("p = %lx, (uint64_t)p =%lx\n", p, (uint64_t)p);
    printf("p = %lx, phdr->p_vaddr =%lx\n", p, phdr->p_vaddr);
    phdr->p_filesz = 0;
    phdr->p_memsz = 0;
    phdr->p_flags = PF_R | PF_W | PF_X;
    phdr->p_align = 0x1000;

    p += (sizeof(Elf64_Ehdr) + sizeof(Elf64_Phdr));
    p = (char *)(((uint64_t)p + ((1 << 12) - 1)) & (~((1 << 12) - 1)));
    printf("p = %lx, &phdr->p_vaddr =%lx, phdr->p_vaddr =%lx\n", p,&phdr->p_vaddr, phdr->p_vaddr);

    printf("The base address of sections is %p\n", p);
    for(int i=0;i < n_objfiles;i++){
        printf("objfile[%d] (%s)\n", i, objfiles[i].filename);
        printf("memcpy(%p, %p, %x)\n", p, objfiles[i].address, objfiles[i].size);
        memcpy(p, objfiles[i].address, objfiles[i].size);

        printf("memcpy done.\n");
        objfiles[i].address = p;
        p += objfiles[i].size;
        p = (char *)(((uint64_t) p + 15) & ~15); Elf64_Shdr *bss = get_section((Elf64_Ehdr *)objfiles[i].address, ".bss");
        if(bss){
            bss->sh_offset = objfiles[i].size;
            memset(p, 0, bss->sh_size);
            p += bss->sh_size;
        }
            p = (char *)(((uint64_t) p + 15) & ~15);
        }
        char *start_filename;
        char *start_address;
    search_symbol(n_objfiles, objfiles, "_start", &start_filename, &start_address);
    ehdr->e_entry = (Elf64_Addr)start_address;
    printf("p = %p, phdr->p_vaddr =%p\n", p, phdr->p_vaddr);
    phdr->p_filesz = (Elf64_Xword)(p - phdr->p_vaddr);
    phdr->p_memsz= (Elf64_Xword)(p - phdr->p_vaddr);

    FILE *fp = fopen("a.out", "wb");
    printf("phdr->p_filesz = %ld\n", phdr->p_filesz);
    fwrite((char *)phdr->p_vaddr, phdr->p_filesz, 1, fp);
    fclose(fp);

    return 0;
}
