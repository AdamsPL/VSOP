#include <stdlib.h>

int main()
{
	char buf[64];
	uint32 next;
	uint32 screen;
	int iter = 0;

	uint64 start, stop;

	register_process("token0");

	while((screen = pidof("sys.drivers.screen")) == -1)
		wait(100);

	while((next = pidof("token1")) == -1)
		wait(100);

	start = time();
	while(1)
	{
		kprintf(buf, "TOKEN!");
		write(next, (uint8*)buf, 32);
		read((uint8*)buf, 32);
		iter++;
		if (iter % 2048 == 0)
		{
			stop = time();
			kprintf(buf, "messages: %i time:%i", iter, stop - start);
			write(screen, (uint8*)buf, 32);
		}
		
	}

	return 0;
}
