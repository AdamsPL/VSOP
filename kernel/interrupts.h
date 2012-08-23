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

#define INT_OK 1
#define INT_ERROR 0
#define INT_SCHED_TICK 128

struct thread_state{
	uint32 ds;
	uint32 edi, esi, ebp, esp, ebx, edx, ecx, eax;
	uint32 int_id, err;
	uint32 eip, cs, eflags, useresp, ss;
};

void regs_init(struct thread_state *regs, uint32 stack, uint32 entry);
void regs_print(struct thread_state *regs);

void lapic_set(uint32 reg, uint32 value);
int lapic_get(uint32 reg);

typedef uint8 (*interrupt_handler)(struct thread_state *state);

void interrupts_init();
void interrupts_register_handler(uint8 int_id, interrupt_handler handler);
void interrupts_start();

void isr_init();
void idt_set(uint16 id, uint32 base, uint8 flags);

#endif
