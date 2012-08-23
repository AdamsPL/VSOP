#include "interrupts.h"

#define ISR(x) extern void _isr##x();
#define ISR_IGNORE(x)
#include "isr_list.h"
ISR(_null)
#undef ISR_IGNORE
#undef ISR


void isr_init()
{
#define ISR(x) idt_set(x, (uint32)_isr##x, 0x8E);
#define ISR_IGNORE(x) idt_set(x, (uint32)_isr_null, 0x8E);
#include "isr_list.h"
#undef ISR_IGNORE
#undef ISR
}
