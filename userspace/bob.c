#include <stdlib.h>

int main()
{
	/*
	descr screen;
	*/
	descr alice;
	char buf[32];
	int i = 0;

	register_process("bob");

	while((alice = connect("alice")) == -1)
		wait(100);
	/*
	while((screen = connect("sys.drivers.screen")) == -1)
		wait(100);
	*/
	kprintf(buf, "HELLO!");
	write(alice, (uint8*)buf, 32);


	while(1)
	{
		alice = select();
		read(alice, (uint8*)buf, 32);
		write(alice, (uint8*)buf, 32);
		/*
		write(screen, (uint8*)buf, 32);
		*/
		exit(i++);
	}

	return 0;
}
