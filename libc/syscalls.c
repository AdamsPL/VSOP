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
	syscall(SYS_CALL_EXIT);
}

stream connect(int pid)
{
	asm volatile("movl %0, %%ebx" :: "m"(pid) : "%ebx");
	return syscall(SYS_CALL_CONNECT);
}

int send(stream str, char *buf, int length)
{
	asm volatile("movl %0, %%ebx" :: "m"(str) : "%ebx");
	asm volatile("movl %0, %%ecx" :: "m"(buf) : "%ecx");
	asm volatile("movl %0, %%edx" :: "m"(length) : "%edx");
	return syscall(SYS_CALL_SEND);
}

stream receive(int *from, char *buf, int length)
{
	asm volatile("movl %0, %%ebx" :: "m"(from) : "%ebx");
	asm volatile("movl %0, %%ecx" :: "m"(buf) : "%ecx");
	asm volatile("movl %0, %%edx" :: "m"(length) : "%edx");
	return syscall(SYS_CALL_RECEIVE);
}

void *mmap(void *addr)
{
	asm volatile("movl %0, %%ebx" :: "m"(addr) : "%ebx");
	return (void*)syscall(SYS_CALL_MMAP);
}

stream proc_register(char *str, int irq)
{
	asm volatile("movl %0, %%ebx" :: "m"(str) : "%ebx");
	asm volatile("movl %0, %%ecx" :: "m"(irq) : "%ecx");
	return syscall(SYS_CALL_REGISTER);
}

stream proc_query(char *str)
{
	asm volatile("movl %0, %%ebx" :: "m"(str) : "%ebx");
	return syscall(SYS_CALL_QUERY);
}

