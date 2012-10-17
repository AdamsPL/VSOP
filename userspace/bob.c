#include <stdlib.h>

int main()
{
	uint32 alice;
	uint32 screen;
	
	char buf[32];

	register_process("bob");

	while((alice = pidof("alice")) == -1)
		wait(100);

	while((screen = pidof("sys.drivers.screen")) == -1)
		wait(100);

	kprintf(buf, "HELLO!");

	write(alice, (uint8*)buf, 32);

	while(1)
	{
		read((uint8*)buf, 32);
		write(alice, (uint8*)buf, 32);
		write(screen, (uint8*)buf, 32);
	}

	return 0;
}
