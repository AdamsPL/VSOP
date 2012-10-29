#include <stdlib.h>

int main()
{
	int screen;
	char buf[32];

	while((screen = connect("sys.drivers.screen")) == -1)
		wait(100);

	kprintf(buf, "ping!");
	while(1)
		write(screen, (uint8*)buf, sizeof(buf));


	return 0;
}
