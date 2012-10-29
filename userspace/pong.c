#include <stdlib.h>

int main()
{
	int screen;

	while((screen = connect("sys.drivers.screen") == -1)
		wait(100);
	
	while(1);
}
