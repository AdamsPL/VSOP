.section .text
.code16

.global _cpu_trampoline
.global _cpu_trampoline_end
.extern gdtr
.extern kpage_dir
.extern cpu_stack
.extern hello_world
.extern iptr

_cpu_trampoline:
	cli

	movl $0x5000, %ebx

	xorl %eax, %eax
	mov %cs, %ax
	add $0x100, %ax
	mov %ax, %ds
	#mov %cs, %ax
	#shl $4, %eax
	xorl %eax, %eax
	movl %eax, %ecx
	movl (%ecx), %ecx
	mov %ecx, (%ebx)
	lgdt (%eax)

	xorl %eax, %eax
	mov $0x10, %ax
	mov %ax, %ds
	mov %ax, %es
	mov %ax, %fs
	mov %ax, %gs
	mov %ax, %ss

	movl %cr0, %eax
	or $0x01, %eax
	movl %eax, %cr0
	jmpl $0x08, $bootstrap
.code32
bootstrap:
	mov $0x10, %ax
	mov %ax, %ds
	mov %ax, %es
	mov %ax, %fs
	mov %ax, %gs
	mov %ax, %ss
	movl $kpage_dir, %eax
	subl $0xC0000000, %eax
	movl %eax, %cr3
	movl %cr0, %eax
	or $0x80000000, %eax
	movl %eax, %cr0
	movl $gdtr, %eax
	lgdt (%eax)
	movl $iptr, %eax
	lidt (%eax)
	mov $0x10, %ax
	mov %ax, %ds
	mov %ax, %es
	mov %ax, %fs
	mov %ax, %gs
	mov %ax, %ss
	jmpl $0x08, $orig_gdt
orig_gdt:
	movl $cpu_stack, %eax
	movl (0xfee00020), %ebx
	shr $24, %ebx
	andl $0x0F, %ebx
	shl $0x2, %ebx
	addl %ebx, %eax
	movl (%eax), %eax
	movl %eax, %esp
	movl %eax, %ebp
	call hello_world
ready_loop:
	hlt
	jmp ready_loop
_cpu_trampoline_end:
