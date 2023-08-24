#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <stdint.h>
#include <sys/mman.h>


#include <sys/types.h>
#include <unistd.h>

// Format of an ELF executable file

#define ELF_MAGIC 0x464C457FU  // "\x7FELF" in little endian

// File header
struct elfhdr {
  unsigned int magic;  // must equal ELF_MAGIC
  unsigned char elf[12];
  unsigned short type;
  unsigned short machine;
  unsigned int version;
  unsigned int entry;
  unsigned int phoff;
  unsigned int shoff;
  unsigned int flags;
  unsigned short ehsize;
  unsigned short phentsize;
  unsigned short phnum;
  unsigned short shentsize;
  unsigned short shnum;
  unsigned short shstrndx;
};

// Program section header
struct proghdr {
  unsigned int type;
  unsigned int off;
  unsigned int vaddr;
  unsigned int paddr;
  unsigned int filesz;
  unsigned int memsz;
  unsigned int flags;
  unsigned int align;
};

struct sechdr{
  unsigned int  name;
  unsigned int  type;
  unsigned int  flags;
  unsigned int  addr;
  unsigned int  offset;
  unsigned int  size;
  unsigned int  link;
  unsigned int  info;
  unsigned int  addralign;
  unsigned int  entsize;
};

typedef struct {
  unsigned int    r_offset;
  unsigned int    r_info;
} Elf32_Rel;

// Values for Proghdr type
#define ELF_PROG_LOAD           1

// Flag bits for Proghdr flags
#define ELF_PROG_FLAG_EXEC      1
#define ELF_PROG_FLAG_WRITE     2
#define ELF_PROG_FLAG_READ      4

struct sechdr shdr_array[19];

int main(int argc, char* argv[]) {
    struct elfhdr elf;
    struct proghdr ph;
    struct sechdr sh;
    int (*sum)(int a, int b);
    void *entry = NULL;
    char *seg = NULL;
    char *dat = NULL;
    int ret; 

    /* Add your ELF loading code here */
    FILE* file = fopen(argv[1], "rb");
    if (!file) {
        printf("File cannot be opened\n");
        return -1;
    }
    fread(&elf, 1, sizeof(elf), file);
    int entries = elf.phnum;
    int sh_entries = elf.shnum;
    

    for (int i = 0; i < entries; i++) {

        fread(&ph, 1, sizeof(ph), file);
        if (ph.type == ELF_PROG_LOAD) {
            seg = mmap(NULL, ph.memsz, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_ANONYMOUS | MAP_PRIVATE, 0, 0);
            fseek(file, ph.off, SEEK_SET);
            fread(seg, 1, ph.memsz, file);
            entry = seg + elf.entry - ph.vaddr; // In our case elf.entry and ph.vaddr both are 0
        }
        
    }

    size_t data_size = 0;
    for (int i = 0; i < sh_entries; i++) {
        fseek(file, elf.shoff + i * sizeof(sh), SEEK_SET);
        fread(&sh, 1, sizeof(sh), file);
        shdr_array[i] = sh;
        if (sh.name != 51) { // 51 is the value for .data
            continue;
        }
        data_size = sh.size;
        dat = mmap(NULL, data_size, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_ANONYMOUS | MAP_PRIVATE, 0, 0);
        fseek(file, sh.offset, SEEK_SET);
        fread(dat, 1, data_size, file);
        
    }
    struct sechdr text = shdr_array[2];
    Elf32_Rel *reltab = malloc(sizeof(Elf32_Rel));
    fseek(file, text.offset, SEEK_SET);
    fread(reltab, 1, sizeof(Elf32_Rel), file);
    int offset = reltab->r_offset;
    unsigned int addr = (unsigned int) dat;
    // making changes after seeing obj dump
    seg[offset + 3] = (addr >> 24) & 0xff;
    seg[offset + 2] = (addr >> 16) & 0xff;
    seg[offset + 1] = (addr >> 8) & 0xff;
    seg[offset] = (addr) & 0xff;
    
    if (entry != NULL) {
        sum = entry; 
        ret = sum(1, 2);
        printf("sum:%d\n", ret); 
    };

}
