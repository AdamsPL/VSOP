#include <stdlib.h>

int main()
{
	char msg[64];
	char buf[128];
	int next;
	int screen;
	int iter = 0;
	int from;
	
	int count = 1;

	const int step = 4096 * count;
	/*
	const int step = 2048;
	const int step = 1024;
	*/
	uint64 start, stop;

	register_process("token0");

	while((screen = connect("sys.drivers.screen")) == -1)
		wait(100);

	kprintf(buf, "STARTED");
	write(screen, (uint8*)buf, 64);

	while((next = connect("token1")) == -1)
		wait(100);

	start = time();
	kprintf(buf, "TOKEN!");
	while(count-- > 0)
		write(next, (uint8*)buf, 32);
	from = select();
	while(1)
	{
		read(from, (uint8*)buf, 32);
		write(next, (uint8*)buf, 32);

		iter++;
		if (iter % step == 0)
		{
			stop = time();
			kprintf(msg, "messages: %i time:%i [%s]", iter, stop - start, buf);
			write(screen, (uint8*)msg, 64);
		}
	}

	return 0;
}
