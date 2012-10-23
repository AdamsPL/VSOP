#include <stdlib.h>

int main()
{
	char buf[64];
	uint32 next;

	register_process("token1");

	while((next = pidof("token2")) == -1)
		wait(100);

	while(1)
	{
		read((uint8*)buf, 32);
		write(next, (uint8*)buf, 32);
	}

	return 0;
}
