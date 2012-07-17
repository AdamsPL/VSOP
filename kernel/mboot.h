#ifndef _MBOOT_H
#define _MBOOT_H

#include "config.h"

struct mboot;

struct mmap {
	uint32 size;
	uint32 base_low;
	uint32 base_high;
	uint32 length_low;
	uint32 length_high;
	uint32 type;
};

void mboot_parse(struct mboot *mboot);
void mboot_load_modules(struct mboot *mboot);
#endif
