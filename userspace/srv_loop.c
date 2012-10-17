#include <stdlib.h>

int main()
{
	int x = 0;
	while(1)
	{
		++x;
		if (x % 1024 == 0)
			exit(x);
	}
	return 0;
}
