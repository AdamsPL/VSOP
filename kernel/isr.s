
.macro ISR_NEC id
	.global _isr\id
	_isr\id:
		cli
		push $0
		push $\id
		jmp isr_common
.endm

.macro ISR_EC id
	.global _isr\id
	_isr\id:
		push $\id
		jmp isr_common
.endm


.macro IRQ id
	.global _irq\id
	_irq\id:
		push $0
		push $\id
		jmp irq_common
.endm

.macro INT_ENTER
	pusha
	mov %ds, %ax
	push %eax

	mov $0x10, %ax
	mov %ax, %ds
	mov %ax, %es
	mov %ax, %fs
	mov %ax, %gs
.endm

.macro INT_LEAVE
	pop %eax
	mov %ax, %ds
	mov %ax, %es
	mov %ax, %fs
	mov %ax, %gs

	popa
	add $8, %esp
	iret
.endm

ISR_NEC 0
ISR_NEC 1
ISR_NEC 2
ISR_NEC 3
ISR_NEC 4
ISR_NEC 5
ISR_NEC 6
ISR_NEC 7
ISR_EC 8
ISR_NEC 9
ISR_EC 10
ISR_EC 11
ISR_EC 12
ISR_EC 13
ISR_EC 14
ISR_NEC 15
ISR_NEC 16
ISR_NEC 17
ISR_NEC 18
ISR_NEC 19
ISR_NEC 20
ISR_NEC 21
ISR_NEC 22
ISR_NEC 23
ISR_NEC 24
ISR_NEC 25
ISR_NEC 26
ISR_NEC 27
ISR_NEC 28
ISR_NEC 29
ISR_NEC 30
ISR_NEC 31

ISR_NEC 80

IRQ 219
IRQ 218
IRQ 217
IRQ 216
IRQ 215
IRQ 214
IRQ 213
IRQ 212
IRQ 211
IRQ 210
IRQ 209
IRQ 208
IRQ 207
IRQ 206
IRQ 205
IRQ 204
IRQ 203
IRQ 202
IRQ 201
IRQ 200

IRQ 128

.extern isr_handler
.extern irq_handler

isr_common:
	INT_ENTER
	call isr_handler
	INT_LEAVE

irq_common:
	INT_ENTER
	call irq_handler
	INT_LEAVE
