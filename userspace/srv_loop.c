#include <stdlib.h>

int main()
{
	register_process("loop");
	while(1)
	{
		exit(0);
		wait(1000);
	}
	return 0;
}
