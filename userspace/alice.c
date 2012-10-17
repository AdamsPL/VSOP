#include <stdlib.h>

int main()
{
	char buf[32];
	descr bob = -1;

	register_process("alice");

	while((bob = pidof("bob")) == -1)
		wait(100);

	while(1)
	{
		read((uint8*)buf, 32);
		write(bob, (uint8*)buf, 32);
	}

	return 0;
}
