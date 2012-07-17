#ifndef _INTERRUPTS_H
#define _INTERRUPTS_H

#include "config.h"

#define INT_RETURN asm("leave;iret;");

#define LAPIC_ID        0x0020
#define LAPIC_TPR       0x0080
#define LAPIC_EOI       0x00B0
#define LAPIC_SIR       0x00F0
#define LAPIC_ERR       0x0280
#define LAPIC_TIMER     0x0320
#define LAPIC_PERF_MON  0x0340
#define IOAPIC_REDIR_BASE  0x10

#define LAPIC_BASE         0xFEE00000
#define IOAPIC_BASE        0xFEC00000

struct thread_state{
	uint32 ds;
	uint32 edi, esi, ebp, esp, ebx, edx, ecx, eax;
	uint32 int_id, err;
	uint32 eip, cs, eflags, useresp, ss;
};

void regs_init(struct thread_state *regs, uint32 stack, uint32 entry);
void regs_print(struct thread_state *regs);

void idt_init();

void ioapic_init();

void apic_init();
void lapic_set(uint32 reg, uint32 value);
int lapic_get(uint32 reg);

void int_handler(struct thread_state regs);
void irq_handler(struct thread_state regs);

typedef void (*handler)(struct thread_state regs);

#endif
