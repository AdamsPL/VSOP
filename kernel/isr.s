.extern eoi
.extern irq_common

.global _leave_kernel
.global _isr_null

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
		cli
		push $\id
		jmp isr_common
.endm


_isr_null:
	iret

isr_common:
	pusha
	mov %ds, %ax
	push %eax

	mov $0x10, %ax
	mov %ax, %ds
	mov %ax, %es
	mov %ax, %fs
	mov %ax, %gs

	call irq_handler

	pop %eax
	mov %ax, %ds
	mov %ax, %es
	mov %ax, %fs
	mov %ax, %gs

	popa
	add $8, %esp
	iret


_leave_kernel:
	push $0x80
	call eoi
	pop %eax
	pop %eax
	pop %eax
	pop %eax
	pop %eax
	mov %ax, %ds
	mov %ax, %es
	mov %ax, %fs
	mov %ax, %gs

	popa
	add $8, %esp
	iret

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

ISR_NEC 80
ISR_NEC 128

ISR_NEC 200
ISR_NEC 201
ISR_NEC 202
ISR_NEC 203
ISR_NEC 204
ISR_NEC 205
ISR_NEC 206
ISR_NEC 207
ISR_NEC 208
ISR_NEC 209
ISR_NEC 210
ISR_NEC 211
ISR_NEC 212
ISR_NEC 213
ISR_NEC 214
ISR_NEC 215
ISR_NEC 216
ISR_NEC 217
ISR_NEC 218
ISR_NEC 219
