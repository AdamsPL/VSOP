#ifndef _UTIL_H
#define _UTIL_H

#include "config.h"

char *kprintf(char *str, const char *format, ...);

int kstrcmp(char *str1, char *str2);

void *kmemcpy(uint8 *destination, const uint8 *source, uint32 count);
void *kmemset(uint8 *ptr, uint8 value, uint32 count);
void *kmemset16(uint16 *ptr, uint16 value, uint32 count);
void *kmemset32(uint32 *ptr, uint32 value, uint32 count);

void *kstrncpy(uint8 *destination, const uint8 *source, uint32 count);
#endif
