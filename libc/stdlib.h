#ifndef _STDLIB_H
#define _STDLIB_H

#include "../kernel/config.h"

#include "syscalls.h"

void *memcpy(uint8 *dest, const uint8 *src, uint32 count);
void *memset(uint8 *dest, uint8 value, uint32 count);
void *memset16(uint16 *ptr, uint16 value, uint32 count);

uint8 port_read8(uint16 port);
void port_write8(uint16 port, uint8 value);

int int2str(char *str, unsigned int n, int base);
char *kprintf(char *str, const char *format, ...);

int strlen(char *str);


#endif
