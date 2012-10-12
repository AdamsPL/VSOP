#include <stdlib.h>

int main()
{
	descr bob;
	char buf[32];
	register_process("alice");

	while(1)
	{
		bob = select();
		read(bob, (uint8*)buf, 32);
		write(bob, (uint8*)buf, 32);
	}

	return 0;
}
