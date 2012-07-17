#ifndef _PORTS_H
#define _PORTS_H

#include "config.h"

#define VGA_PORT_PREPARE 0x3D4
#define VGA_PORT_SEND 0x3D5
#define VGA_HIGH_CURSOR 14
#define VGA_LOW_CURSOR 15

inline static void port_write(uint16 port, uint8 value){
	asm volatile ("outb %1, %0" : : "dN"(port), "a"(value));
}

inline static uint8 port_read_8(uint16 port){
	uint8 ret;
	asm volatile("inb %1, %0" : "=a"(ret) : "dN"(port));
	return ret;
}

inline static uint16 port_read_16(uint16 port){
	uint16 ret;
	asm volatile("inw %1, %0" : "=a"(ret) : "dN"(port));
	return ret;
}
#endif
