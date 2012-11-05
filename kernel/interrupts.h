#ifndef _INTERRUPTS_H
#define _INTERRUPTS_H

#include "config.h"

#define INT_RETURN asm("leave;iret;");

#define LAPIC_ID        	0x0020
#define LAPIC_TPR       	0x0080
#define LAPIC_EOI       	0x00B0
#define LAPIC_SIR       	0x00F0
#define LAPIC_ERR       	0x0280
#define LAPIC_TIMER     	0x0320
#define LAPIC_PERF_MON  	0x0340
#define LAPIC_ICR_LOW       0x0300
#define LAPIC_ICR_HIGH      0x0310
#define LAPIC_INT0  	    0x0350
#define LAPIC_INT1	    	0x0360
#define LAPIC_EINT	    	0x0370
#define IOAPIC_REDIR_BASE  	0x10

#define LAPIC_BASE         	0xFEE00000
#define IOAPIC_BASE        	0xFEC00000

#define INT_OK 1
#define INT_ERROR 0
#define INT_SCHED_TICK 128
#define INT_SYSCALL 80
#define INT_RTC 211

#define PREEMPT_DISABLE		0xFF

struct thread_state{
	uint32 tpr;
	uint32 ds;
	uint32 edi, esi, ebp, esp, ebx, edx, ecx, eax;
	uint32 int_id, err;
	uint32 eip, cs, eflags, useresp, ss;
};

enum thread_flags
{
	THREAD_KERNEL = 0,
	THREAD_USERSPACE
};

void regs_init(struct thread_state *regs, uint32 stack, uint32 entry, enum thread_flags flags);
void regs_print(struct thread_state *regs);

void lapic_set(uint32 reg, uint32 value);
int lapic_get(uint32 reg);

typedef uint8 (*interrupt_handler)(struct thread_state *state);

void interrupts_init(void);
void interrupts_register_handler(uint8 int_id, interrupt_handler handler);
void interrupts_start(void);

void isr_init(void);
void idt_set(uint16 id, uint32 base, uint8 flags);

void eoi(int id);

void ioapic_init(void);
void lapic_init(void);
void apic_enable(void);

void interrupts_disable();
void interrupts_enable();

int is_preemptible();
void preempt_disable();
void preempt_enable();

#endif
