.code32

.set MBOOT_PAGE_ALIGN, 1<<0
.set MBOOT_MEM_INFO, 1<<1
.set MBOOT_HEADER_MAGIC, 0x1BADB002
.set MBOOT_HEADER_FLAGS, (MBOOT_PAGE_ALIGN | MBOOT_MEM_INFO)
.set MBOOT_CHECKSUM, -(MBOOT_HEADER_MAGIC + MBOOT_HEADER_FLAGS)

.set STACKSIZE,           0x1000

.section .__mbootheader

.global mboot
.extern code
.extern bss
.extern end

.align 0x4
mboot:
.long MBOOT_HEADER_MAGIC
.long MBOOT_HEADER_FLAGS
.long MBOOT_CHECKSUM
.long mboot
.long code
.long bss
.long end
.long start

.section .text

.global start
.extern kmain

start:
	cli
	lgdt (fake_gdt_ptr - 0xC0000000)
	movl $0x10, %ecx
	movw %cx, %ds
	movw %cx, %es
	movw %cx, %fs
	movw %cx, %gs
	movw %cx, %ss
	jmp $0x08, $tmp_page_dir

tmp_page_dir:
	mov $(stack + STACKSIZE), %esp
	push %eax
	push %ebx
	movl $0, %ecx
	movl $kpage_dir, %eax
	movl $kpage_table, %ebx

loop:
	movl $0x00, (%eax)
	movl %ecx, %edx
	or $7, %edx
	movl %edx, (%ebx)
	
	addl $4, %eax
	addl $4, %ebx
	addl $0x1000, %ecx
	cmpl $0x400000, %ecx
	jl loop

	movl $kpage_table - 0xC0000000, %eax;
	or $0x07, %eax;
	movl %eax, kpage_dir
	movl %eax, kpage_dir + 768*4

	movl $kpage_dir - 0xC0000000, %eax;
	or $0x07, %eax;
	movl %eax, kpage_dir + 1023*4

	movl $kpage_dir - 0xC0000000, %eax
	movl %eax, %cr3

	movl %cr0, %eax
	or $0x80000000, %eax
	movl %eax, %cr0

call_kernel:
	call kmain

.global fake_gdt_ptr
.global fake_gdt_end

fake_gdt_ptr:
	.word fake_gdt_end - fake_gdt - 1
	.long fake_gdt - 0xC0000000
fake_gdt:
	.long 0, 0
	.byte 0xFF, 0xFF, 0x0, 0x0, 0x0, 0x9A, 0xCF, 0x40
	.byte 0xFF, 0xFF, 0x0, 0x0, 0x0, 0x92, 0xCF, 0x40
fake_gdt_end:

.section .data

.align 0x1000
.global kpage_dir
.global kpage_table

kpage_dir:
	.skip 0x1000
kpage_table:
	.skip 0x1000
stack:
	.skip STACKSIZE
