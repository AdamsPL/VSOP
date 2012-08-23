#include "screen.h"
#include "memory.h"
#include "util.h"
#include "cpu.h"
#include "interrupts.h"
#include "paging.h"
#include "locks.h"
#include "ipc.h"
#include "ports.h"
#include "multitasking.h"
#include "gdt.h"
#include "elf.h"
#include "drivers.h"

void kmain(struct mboot *mboot, unsigned int magic)
{
	gdt_init();
	screen_clear();
	mboot_parse(mboot);
	interrupts_init();
	drivers_init();
	sched_init_all();

	port_write(0x70, 0x0B);
	char prev = port_read_8(0x71);
	port_write(0x70, 0x0B);
	port_write(0x71, prev | 0x40);

	interrupts_start();

	mboot_load_modules(mboot);

/*
	mpc = mp_find()->config;
	
	ptr = (uint8*)mpc + sizeof(*mpc);
	for (i = 0; i < mpc->entry_count; ++i) {
		if (*ptr == 0){
			struct MP_proc_entry *pe_ptr = (struct MP_proc_entry*)ptr;
			screen_putstr(kprintf(buf, "lapic id:%i ", pe_ptr->lapic_id));
			screen_putstr(kprintf(buf, "cpu_flags:%x\n", pe_ptr->cpu_flags));
			ptr += 20;
		}else
			break;
	}
*/
	while(1)
		asm("hlt");
}
