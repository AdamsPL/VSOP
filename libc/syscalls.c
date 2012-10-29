#include "stdlib.h"

#include "../kernel/system_calls.h"

static int syscall(int number)
{
	int result;

	asm volatile("movl %0, %%eax" :: "m"(number));
	asm volatile("int $80");
	asm volatile("movl %%eax, %0" : "=m"(result));

	return result;
}

int exit(int error_code)
{
	asm volatile("movl %0, %%ebx" :: "m"(error_code) : "%ebx");
	return syscall(SYSCALL_EXIT);
}

int wait(int ticks)
{
	asm volatile("movl %0, %%ebx" :: "m"(ticks) : "%ebx");
	return syscall(SYSCALL_WAIT);
}

int register_process(const char *name)
{
	asm volatile("movl %0, %%ebx" :: "m"(name) : "%ebx");
	return syscall(SYSCALL_REGISTER); 
}

int read(int descr, const uint8 *buf, const uint32 size)
{
	asm volatile("movl %0, %%ebx" :: "m"(descr) : "%ebx");
	asm volatile("movl %0, %%ecx" :: "m"(buf) : "%ecx");
	asm volatile("movl %0, %%edx" :: "m"(size) : "%edx");
	return syscall(SYSCALL_READ);
}

int write(int descr, const uint8 *buf, const uint32 size)
{
	asm volatile("movl %0, %%ebx" :: "m"(descr) : "%ebx");
	asm volatile("movl %0, %%ecx" :: "m"(buf) : "%ecx");
	asm volatile("movl %0, %%edx" :: "m"(size) : "%edx");
	return syscall(SYSCALL_WRITE);
}

int select()
{
	return syscall(SYSCALL_SELECT);
}

int connect(const char *name)
{
	asm volatile("movl %0, %%ebx" :: "m"(name) : "%ebx");
	return syscall(SYSCALL_CONNECT);
}

void mmap(void *virt_addr, void *phys_addr)
{
	asm volatile("movl %0, %%ebx" :: "m"(virt_addr) : "%ebx");
	asm volatile("movl %0, %%ecx" :: "m"(phys_addr) : "%ecx");
	syscall(SYSCALL_MMAP);
}

void handle(int irq)
{
	asm volatile("movl %0, %%ebx" :: "m"(irq) : "%ebx");
	syscall(SYSCALL_HANDLE);
}

uint8 peek()
{
	return syscall(SYSCALL_PEEK);
}

uint64 time()
{
	return syscall(SYSCALL_TIME);
}
