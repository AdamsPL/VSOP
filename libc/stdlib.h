#ifndef _STDLIB_H
#define _STDLIB_H

typedef int stream;
typedef unsigned char uint8;
typedef unsigned short uint16;
typedef unsigned int uint32;

void exit(int error_code);
stream connect(int pid);
int send(stream str, char *buf, int length);
stream receive(int *from, char *buf, int length);
void *mmap(void *addr);
stream proc_register(char *str, int irq);
stream proc_query(char *str);

void *memcpy(uint8 *dest, const uint8 *src, uint32 count);
void *memset(uint8 *dest, uint8 value, uint32 count);
void *memset16(uint16 *ptr, uint16 value, uint32 count);

uint8 port_read8(uint16 port);
void port_write8(uint16 port, uint8 value);

int int2str(char *str, unsigned int n, int base);
char *printf(char *str, const char *format, ...);

int strlen(char *str);
#endif
