#include <stdlib.h>

#include "streamer_common.h"

int main()
{
	int screen;
	int in;
	char buf[PACKET_SIZE];
	int i;
	int t;

	register_process("streamer_out");

	while((screen = connect("sys.drivers.screen")) == -1)
		wait(100);

	in = select();
	for (t = 0; t < TESTS; ++t)
	{
		for (i = 0; i < PACKETS; ++i)
		{
			read(in, (uint8*)buf, sizeof(buf));
		}
		kprintf(buf, "%i : STOP", time());
		write(screen, (uint8*)buf, 32);
		write(in, (uint8*)buf, sizeof(buf));
	}

	return 0;
}
