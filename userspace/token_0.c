#include <stdlib.h>

int main()
{
	char buf[64];
	int next;
	int screen;
	int iter = 0;
	int from;

	uint64 start, stop;

	register_process("token0");

	while((screen = connect("sys.drivers.screen")) == -1)
		wait(100);

	while((next = connect("token1")) == -1)
		wait(100);

	kprintf(buf, "STARTED");
	write(screen, (uint8*)buf, 64);

	start = time();
	kprintf(buf, "TOKEN!");
	write(next, (uint8*)buf, 32);
	from = select();
	while(1)
	{
		read(from, (uint8*)buf, 32);
		write(next, (uint8*)buf, 32);

		iter++;
		if (iter % 2048 == 0)
		{
			stop = time();
			kprintf(buf, "messages: %i time:%i", iter, stop - start);
			write(screen, (uint8*)buf, 64);
		}
	}

	return 0;
}
