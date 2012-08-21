#ifndef _ELF_H
#define _ELF_H

#include "config.h"

#define EI_NIDENT 16

struct elf_header{
	unsigned char e_ident[EI_NIDENT];
	uint16 e_type;
	uint16 e_machine;
	uint32 e_version;
	uint32 e_entry;
	uint32 e_phoff;
	uint32 e_shoff;
	uint32 e_flags;
	uint16 e_ehsize;
	uint16 e_phentsize;
	uint16 e_phnum;
	uint16 e_shentsize;
	uint16 e_shnum;
	uint16 e_shtrndx;
};

struct elf_section{
	uint32 sh_name;
	uint32 sh_type;
	uint32 sh_flags;
	uint32 sh_addr;
	uint32 sh_offset;
	uint32 sh_size;
	uint32 sh_link;
	uint32 sh_info;
	uint32 sh_addralign;
	uint32 sh_entsize;
};

pid_t elf_load(uint8 *buf);

#endif
