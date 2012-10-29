#include <stdlib.h>

#define PACKET_SIZE 128
#define PAYLOAD 4096000
#define PACKETS (PAYLOAD / PACKET_SIZE)

int main()
{
	int screen;
	int in;
	char buf[PACKET_SIZE];
	int i;

	register_process("streamer_out");

	while((screen = connect("sys.drivers.screen")) == -1)
		wait(100);

	for (i = 0; i < PACKETS; ++i)
	{
		in = select();
		read(in, (uint8*)buf, sizeof(buf));
	}
	kprintf(buf, "%i : STOP", time());
	write(screen, (uint8*)buf, 32);


	return 0;
}
