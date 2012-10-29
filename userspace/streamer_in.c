#include <stdlib.h>

#include "streamer_common.h"

int main()
{
	int screen;
	int out;
	char buf[PACKET_SIZE];
	int i;
	int t;

	while((screen = connect("sys.drivers.screen")) == -1)
		wait(100);

	while((out = connect("streamer_out")) == -1)
		wait(100);

	for (t = 0; t < TESTS; ++t)
	{
		kprintf(buf, "%i : START", time());
		write(screen, (uint8*)buf, 32);
		for (i = 0; i < PACKETS; ++i)
			write(out, (uint8*)buf, sizeof(buf));
		read(out, (uint8*)buf, sizeof(buf));
	}	
	return 0;
}
