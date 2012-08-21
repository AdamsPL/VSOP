#include "interrupts.h"
#include "ports.h"
#include "util.h"
#include "multitasking.h"
#include "syscall.h"
#include "drivers.h"
#include "cpu.h"
#include "locks.h"
#include "memory.h"

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

struct int_handler_elem
{
	interrupt_handler handler;
	struct int_handler_elem *next;
};

struct int_handler_elem *int_handlers[256];

static struct idt_entry idt_entries[256];
static volatile struct idt iptr;

#include "screen.h"

#define PRINT_FIELD(x) screen_putstr(kprintf(buf, #x":%x ", regs->x));

static int base = 0;

void dump_idt()
{
	uint32 i;
	char buf[32];
	for (i = base; i < base+1; ++i){
		screen_putstr(kprintf(buf, "%i %x%x %x %x\n", i, idt_entries[i].base_high, idt_entries[i].base_low, idt_entries[i].selector, idt_entries[i].flags));
	}
	base += 1;
}

void regs_print(struct thread_state *regs)
{
	char buf[32];
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
	int reg_low = IOAPIC_REDIR_BASE + irq;
	int reg_high = reg_low + 1;
	/*disabling, just in case...*/
	ioapic_set(reg_low, IOAPIC_DISABLE);
	ioapic_set(reg_high, 0x00);
	ioapic_set(reg_low, vector);
}

void ioapic_init()
{
	ioapic_map(0, 219);
	ioapic_map(1, 218);
	ioapic_map(2, 217);
	ioapic_map(3, 216);
	/*ioapic_map(4, 215);*/
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
	lapic_set(LAPIC_SIR, 0x0010F);
	lapic_set(LAPIC_TPR, 0x00020);
	lapic_set(LAPIC_TIMER, 0x10000);
}

void apic_init()
{
	uint32 lo, hi, msr;
/*
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
	*/
	/*Disabling 8259*/
	port_write(0xA1, 0xFF);
	port_write(0x21, 0xFF);
	port_write(0x22, 0x70);	
	port_write(0x23, 0x01);	

	/*enabling apic*/
	lo = 0;
	hi = 0;
	msr = 0x1B;
	asm volatile("rdmsr" : "=a"(lo), "=d"(hi) : "c"(msr));
	lo &= 0xFFFFF000;
	lo |= 0x800;
	asm volatile("wrmsr" :: "a"(lo), "d"(hi), "c"(msr));

	ioapic_init();
	lapic_init();
}

static uint8 unhandled_interrupt_handler(struct thread_state *state)
{
	char buf[256];
	uint32 cr2 = 0;
	asm volatile("movl %%cr2, %0" : "=a"(cr2));
	screen_putstr(kprintf(buf, "unhandled int! pid:%i cr2: %x cpu:%i\n", proc_cur(), cr2, cpuid()));
	regs_print(state);
	while(1){
		asm("hlt");
	}
	return INT_OK;
}

static void common_handler(struct thread_state *regs)
{
	char buf[256];
	struct int_handler_elem *elem;

	screen_putstr(kprintf(buf, "common handler\n"));
	thread_save_state(sched_current_thread(), regs);

	elem = int_handlers[regs->int_id];

	while(elem && elem->handler(regs) != INT_OK)
		elem = elem->next;

	/*
	switch(regs->int_id){
		case 128:
			sched_tick(sched_current());
			break;
		case 217:
			server_irq_notify(217);
			break;
		case 80:
			syscall(regs);
			break;
		case 15:
			screen_putstr(kprintf(buf, "SPUR!\n"));
			break;
		default:
	}
	*/
}

void isr_handler(struct thread_state regs)
{
	common_handler(&regs);
}

void irq_handler(struct thread_state regs)
{
	common_handler(&regs);
	lapic_set(LAPIC_EOI, 0x01);
}

#define ISR(x) extern void _isr##x();
#define IDT_ISR(x) idt_set(x, (uint32)_isr##x, 0x8E)
#define IRQ(x) extern void _irq##x();
#define IDT_IRQ(x) idt_set(x, (uint32)_irq##x, 0x8E)

/*cpu exceptions*/
ISR(0) 
ISR(1) 
ISR(2) 
ISR(3) 
ISR(4) 
ISR(5) 
ISR(6) 
ISR(7) 
ISR(8) 
ISR(9) 
ISR(10) 
ISR(11) 
ISR(12) 
ISR(13) 
ISR(14) 
ISR(15) 
ISR(16) 
ISR(17) 
ISR(18) 
ISR(19) 
ISR(20) 
ISR(21) 
ISR(22) 
ISR(23) 
ISR(24) 
ISR(25) 
ISR(26) 
ISR(27) 
ISR(28) 
ISR(29) 
ISR(30) 
ISR(31) 

ISR(80) 
IRQ(128) /*timer*/

/*ioapic + pci*/
IRQ(219) 
IRQ(218) 
IRQ(217) 
IRQ(216) 
IRQ(215) 
IRQ(214) 
IRQ(213) 
IRQ(212) 
IRQ(211) 
IRQ(210) 
IRQ(209) 
IRQ(208) 
IRQ(207) 
IRQ(206) 
IRQ(205) 
IRQ(204) 
IRQ(203) 
IRQ(202) 
IRQ(201) 
IRQ(200) 

void idt_init()
{
	IDT_ISR(0);
	IDT_ISR(1);
	IDT_ISR(2);
	IDT_ISR(3);
	IDT_ISR(4);
	IDT_ISR(5);
	IDT_ISR(6);
	IDT_ISR(7);
	IDT_ISR(8);
	IDT_ISR(9);
	IDT_ISR(10);
	IDT_ISR(11);
	IDT_ISR(12);
	IDT_ISR(13);
	IDT_ISR(14);
	IDT_ISR(15);
	IDT_ISR(16);
	IDT_ISR(17);
	IDT_ISR(18);
	IDT_ISR(19);
	IDT_ISR(20);
	IDT_ISR(21);
	IDT_ISR(22);
	IDT_ISR(23);
	IDT_ISR(24);
	IDT_ISR(25);
	IDT_ISR(26);
	IDT_ISR(27);
	IDT_ISR(28);
	IDT_ISR(29);
	IDT_ISR(30);
	IDT_ISR(31);

	IDT_ISR(80);
	IDT_IRQ(128);

	IDT_IRQ(200);
	IDT_IRQ(201);
	IDT_IRQ(202);
	IDT_IRQ(203);

	IDT_IRQ(219);
	IDT_IRQ(218);
	IDT_IRQ(217);
	IDT_IRQ(216);
	IDT_IRQ(215);
	IDT_IRQ(214);
	IDT_IRQ(213);
	IDT_IRQ(212);
	IDT_IRQ(211);
	IDT_IRQ(210);
	IDT_IRQ(209);
	IDT_IRQ(208);
	IDT_IRQ(207);
	IDT_IRQ(206);
	IDT_IRQ(205);
	IDT_IRQ(204);

	iptr.base = (uint32)VIRT_TO_PHYS(&idt_entries);
	iptr.size = sizeof(idt_entries) - 1;
	idt_flush((uint32)&iptr);
}

void regs_init(struct thread_state *regs, uint32 stack, uint32 entry)
{
	uint32 flags;
	stack += PAGE_SIZE;

	asm("pushf");
	asm("pop %eax");
	asm("movl %%eax, %0" : "=a"(flags));

	kmemset((void*)regs, 0, sizeof(struct thread_state));

	regs->eip = entry;
	regs->ds = 0x23;
	regs->cs = 0x1b;
	regs->ss = regs->ds;
	regs->eflags = flags;
	regs->ebp = (uint32)stack;
	regs->useresp = (uint32)stack;
	regs->esp = (uint32)stack;
}

void interrupts_register_handler(uint8 int_id, interrupt_handler handler)
{
	struct int_handler_elem *elem = NEW(struct int_handler_elem);
	elem->handler = handler;
	elem->next = int_handlers[int_id];
	int_handlers[int_id] = elem;
}

void interrupts_init()
{
	int i;
	idt_init();
	apic_init();
	for (i = 0; i < 256; ++i)
		interrupts_register_handler(i, unhandled_interrupt_handler);
}

void interrupts_start()
{
	asm("sti");
}

void interrupts_stop()
{
	asm("cli");
}
