#include <stdlib.h>

#define PACKET_SIZE 128
#define PAYLOAD 4096000
#define PACKETS (PAYLOAD / PACKET_SIZE)

int main()
{
	int screen;
	int out;
	char buf[PACKET_SIZE];
	int i;

	while((screen = connect("sys.drivers.screen")) == -1)
		wait(100);

	while((out = connect("streamer_out")) == -1)
		wait(100);

	kprintf(buf, "%i : START", time());

	write(screen, (uint8*)buf, 32);
	for (i = 0; i < PACKETS; ++i)
		write(out, (uint8*)buf, sizeof(buf));

	return 0;
}
