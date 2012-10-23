#include "interrupts.h"
#include "ports.h"
#include "util.h"
#include "syscall.h"
#include "drivers.h"
#include "cpu.h"
#include "locks.h"
#include "memory.h"
#include "timer.h"
#include "scheduler.h"
#include "gdt.h"
#include "config.h"

#define IOAPIC_DISABLE     0x10000

struct idt_entry{
	uint16 base_low;
	uint16 selector;
	uint8 zero;
	uint8 flags;
	uint16 base_high;
}__attribute__((packed));

struct idt {
	uint16 size;
	uint32 base;
}__attribute__((packed));

interrupt_handler int_handlers[256];

static struct idt_entry idt_entries[256];
volatile struct idt iptr;

#include "screen.h"

#define PRINT_FIELD(x) screen_putstr(kprintf(buf, #x":%x|", regs->x));

static uint32 cr2 = 0;

void regs_print(struct thread_state *regs)
{
	char buf[256];
	PRINT_FIELD(ds);
	PRINT_FIELD(edi);
	PRINT_FIELD(esi);
	PRINT_FIELD(ebp);
	PRINT_FIELD(esp);
	PRINT_FIELD(ebx);
	PRINT_FIELD(edx);
	PRINT_FIELD(ecx);
	PRINT_FIELD(eax);
	PRINT_FIELD(int_id);
	PRINT_FIELD(err);
	PRINT_FIELD(eip);
	PRINT_FIELD(cs);
	PRINT_FIELD(eflags);
	PRINT_FIELD(useresp);
	PRINT_FIELD(ss);
	screen_putstr(kprintf(buf, "\n"));
}

void idt_set(uint16 id, uint32 base, uint8 flags)
{
	idt_entries[id].base_low = base & 0xFFFF;
	idt_entries[id].base_high = (base >> 16) & 0xFFFF;
	idt_entries[id].selector = 0x08;
	idt_entries[id].zero = 0x00;
	idt_entries[id].flags = flags | 0x60;
}

extern void idt_flush(uint32 ptr);

static void ioapic_set(uint32 reg, uint32 value)
{
	volatile uint32 *io_regsel = (uint32*)IOAPIC_BASE;
	volatile uint32 *io_win = (uint32*)(IOAPIC_BASE + 0x10);

	*io_regsel = reg;
	*io_win = value;
}

static void ioapic_map(uint32 irq, uint32 vector)
{
	int reg_low = IOAPIC_REDIR_BASE + irq*2;
	int reg_high = reg_low + 1;
	/*disabling, just in case...*/
	ioapic_set(reg_low, IOAPIC_DISABLE);
	ioapic_set(reg_high, 0x00);
	/*
	ioapic_set(reg_low, vector);
	*/
	ioapic_set(reg_low, (1 << 8) | vector);
}

uint8 keyboard_handler(struct thread_state *state)
{
	char buf[128];
	struct time_t uptime = timer_uptime();
	screen_putstr(kprintf(buf, "keyboard!: eip:%x d:%i h:%i m:%i s:%i ms:%i\n", state->eip, uptime.days, uptime.hours, uptime.minutes, uptime.sec, uptime.milisec));
	return INT_OK;
}

void ioapic_init()
{
	ioapic_map(0, 219);
	ioapic_map(1, 218);
	/*ioapic_map(2, 217);*/
	ioapic_map(3, 216);
	ioapic_map(4, 215);
	ioapic_map(5, 214);
	ioapic_map(6, 213);
	ioapic_map(7, 212);
	ioapic_map(8, 211);
	ioapic_map(9, 210);
	ioapic_map(10, 209);
	ioapic_map(11, 208);
	ioapic_map(12, 207);
	ioapic_map(13, 206);
	ioapic_map(14, 205);
	ioapic_map(15, 204);

}

void lapic_set(uint32 reg, uint32 value)
{
	volatile uint32 *lapic = (uint32*)(LAPIC_BASE + reg);
	*lapic = value;
}

int lapic_get(uint32 reg)
{
	volatile uint32 *lapic = (uint32*)(LAPIC_BASE + reg);
	return *lapic;
}

void lapic_init()
{
	lapic_set(LAPIC_TPR, 0x00020);
	lapic_set(LAPIC_TIMER, 0x10000);
	lapic_set(LAPIC_PERF_MON, 0x10000);
	lapic_set(LAPIC_INT0, 0x08700);
	lapic_set(LAPIC_INT1, 0x00400);
	lapic_set(LAPIC_ERR, 0x00370);
	lapic_set(LAPIC_SIR, 0x0010F);
}

void apic_enable(void)
{
	uint32 lo, hi, msr;

	/*enabling apic*/
	lo = 0;
	hi = 0;
	msr = 0x1B;
	asm volatile("rdmsr" : "=a"(lo), "=d"(hi) : "c"(msr));
	lo &= 0xFFFFF000;
	lo |= 0x800;
	asm volatile("wrmsr" :: "a"(lo), "d"(hi), "c"(msr));
}

void apic_init()
{
	port_write(0x20, 0x11);
	port_write(0xA0, 0x11);

	port_write(0x21, 0x20);
	port_write(0xA1, 0x28);
	port_write(0x21, 0x04);
	port_write(0xA1, 0x02);
	port_write(0x21, 0x01);
	port_write(0xA1, 0x01);
	port_write(0x21, 0x00);
	port_write(0xA1, 0x00);

	/*Disabling 8259*/
	port_write(0xA1, 0xFF);
	port_write(0x21, 0xFF);
	port_write(0x22, 0x70);	
	port_write(0x23, 0x01);	
	

	apic_enable();
	ioapic_init();
	lapic_init();
}

static uint8 unhandled_interrupt_handler(struct thread_state *state)
{
	char buf[256];
	/*screen_clear();*/
	screen_putstr(kprintf(buf, "unhandled state:%x int(%x)! proc:%x thread: %x cpu:%x\n", state, state->int_id, sched_cur_proc(), sched_cur_thread(), cpuid()));
	regs_print(state);
	return INT_OK;
}

void eoi(int id)
{
	lapic_set(LAPIC_TPR, 0x00);
	if (id > 0x70)
	{
		lapic_set(LAPIC_EOI, 0x01);
	}
}

void irq_handler(struct thread_state regs)
{
	lapic_set(LAPIC_TPR, 0xE0);
	asm volatile("movl %%cr2, %0" : "=a"(cr2));

	if (regs.ds > 0x100)
		unhandled_interrupt_handler(&regs);
	
	int_handlers[regs.int_id](&regs);

	eoi(regs.int_id);
}

void idt_init()
{
	isr_init();
	iptr.base = (uint32)VIRT_TO_PHYS(&idt_entries);
	iptr.size = sizeof(idt_entries) - 1;
	idt_flush((uint32)&iptr);
}

void regs_init(struct thread_state *regs, uint32 stack, uint32 entry, enum thread_flags flags)
{
	kmemset((void*)regs, 0, sizeof(struct thread_state));

	regs->eip = entry;
	if (flags && THREAD_USERSPACE)
	{
		regs->ds = 0x23;
		regs->cs = 0x1b;
	}
	else
	{
		regs->ds = 0x10;
		regs->cs = 0x08;
	}
	regs->ss = regs->ds;
	regs->eflags = FLAGS_INT_ENABLE;
	regs->ebp = (uint32)stack;
	regs->useresp = (uint32)stack;
	regs->esp = (uint32)stack;
}

void interrupts_register_handler(uint8 int_id, interrupt_handler handler)
{
	int_handlers[int_id] = handler;
}

static uint8 ignore_int(struct thread_state *regs)
{
	return INT_OK;
}

void interrupts_init()
{
	int i;
	idt_init();
	apic_init();
	for (i = 0; i < 256; ++i)
		interrupts_register_handler(i, unhandled_interrupt_handler);
	interrupts_register_handler(218, keyboard_handler);
	interrupts_register_handler(15, ignore_int);
	interrupts_register_handler(INT_SCHED_TICK, sched_tick);
}

void interrupts_start()
{
	tss_set_stack(cpuid(), (uint32)kmalloc(PAGE_SIZE));
	tss_flush(cpuid());
	lapic_set(LAPIC_TPR, 0x00);
	asm("sti");
}

void interrupts_stop()
{
	asm("cli");
}
