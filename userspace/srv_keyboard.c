#include <stdlib.h>

uint8 status[256];

void read_key()
{
	uint8 opcode = port_read8(0x60);
	status[opcode & 0x7F] = !(opcode >> 7);
}

int main()
{
	int os;
	int snake;
	char buf[64];
	memset(status, 0, sizeof(status));

	register_process("sys.drivers.keyboard");
	handle(218);

	while((snake = connect("snake")) == -1)
		wait(100);
	/*
	while((snake = connect("sys.drivers.screen")) == -1)
		wait(100);
	*/

	write(snake, (uint8*)status, sizeof(status));

	os = select();

	while(1)
	{
		read(os, (uint8*)buf, 1);
		read_key();
		write(snake, (uint8*)status, sizeof(status));
	}
	return 0;
}
