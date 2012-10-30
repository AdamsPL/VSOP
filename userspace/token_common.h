#ifndef _TOKEN_COMMON_H
#define _TOKEN_COMMON_H

int main()
{
	char buf[64];
	int next;
	int from;

	register_process(NAME);

	while((next = connect(TARGET)) == -1)
		wait(100);

	from = select();
	while(1)
	{
		read(from, (uint8*)buf, 32);
		write(next, (uint8*)buf, 32);
	}

	return 0;
}
#endif
