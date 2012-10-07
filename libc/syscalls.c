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

void exit(int error_code)
{
	asm volatile("movl %0, %%ebx" :: "m"(error_code) : "%ebx");
	syscall(SYSCALL_EXIT);
}

void wait(int ticks)
{
	asm volatile("movl %0, %%ebx" :: "m"(ticks) : "%ebx");
	syscall(SYSCALL_WAIT);
}

int register_process(const char *name)
{
	asm volatile("movl %0, %%ebx" :: "m"(name) : "%ebx");
	return syscall(SYSCALL_REGISTER); 
}

descr connect(const char *name)
{
	asm volatile("movl %0, %%ebx" :: "m"(name) : "%ebx");
	return syscall(SYSCALL_CONNECT);
}

descr select()
{
	return syscall(SYSCALL_SELECT);
}

int read(descr dsc, const uint8 *buf, const uint32 size)
{
	asm volatile("movl %0, %%ebx" :: "m"(dsc) : "%ebx");
	asm volatile("movl %0, %%ecx" :: "m"(buf) : "%ecx");
	asm volatile("movl %0, %%edx" :: "m"(size) : "%edx");
	return syscall(SYSCALL_READ);
}

int write(descr dsc, const uint8 *buf, const uint32 size)
{
	asm volatile("movl %0, %%ebx" :: "m"(dsc) : "%ebx");
	asm volatile("movl %0, %%ecx" :: "m"(buf) : "%ecx");
	asm volatile("movl %0, %%edx" :: "m"(size) : "%edx");
	return syscall(SYSCALL_WRITE);
}

void mmap(void *virt_addr, void *phys_addr)
{
	asm volatile("movl %0, %%ebx" :: "m"(virt_addr) : "%ebx");
	asm volatile("movl %0, %%ecx" :: "m"(phys_addr) : "%ecx");
	syscall(SYSCALL_MMAP);
}
