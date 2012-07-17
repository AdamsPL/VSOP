#include <stdlib.h>

int main()
{
	int pid = -1;
	char buf[128];
	int stream = -1;
	int len = -1;

	while(pid == -1){
		pid = proc_query("sys.drivers.screen");
	}

	stream = connect(pid);

	len = 2;
	memcpy((uint8*)buf, (uint8*)"X", len);
	while(1){
		send(stream, buf, len);
	}
	return 0;
}
