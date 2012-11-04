#ifndef _SYSCALLS_H
#define _SYSCALLS_H

int exit(int error_code);
int wait(int ticks);
int connect(const char *name);
int select();
int register_process(const char *name);

int read(int descr, const uint8 *buf, const uint32 size);
int write(int descr, const uint8 *buf, const uint32 size);

void mmap(void *virt, void *phys);
void handle(int irq);
int peek(int descr);
uint64 time(void);

#endif
