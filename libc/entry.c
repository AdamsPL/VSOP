
extern void main(void);

void _start(void)
{
	asm volatile("and $0xfffffff0, %esp");
	main();
	while(1);
}
