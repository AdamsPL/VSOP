#include "stdlib.h"

void port_write8(uint16 port, uint8 value){
	asm volatile ("outb %1, %0" : : "dN"(port), "a"(value));
}


uint8 port_read8(uint16 port){
	uint8 ret;
	asm volatile("inb %1, %0" : "=a"(ret) : "dN"(port));
	return ret;
}

