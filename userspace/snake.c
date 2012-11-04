#include <stdlib.h>

#define SCREEN_WIDTH 80
#define SCREEN_HEIGHT 25

char buf[SCREEN_WIDTH * SCREEN_HEIGHT];
uint8 status[128];

char symbol= '.';
int iterations = 0;

#define KEY_UP 		0x11
#define KEY_DOWN 	0x1F
#define KEY_LEFT 	0x1E
#define KEY_RIGHT 	0x20

#define MAX_LEN 400

struct pos
{
	short x, y;
};

struct pos snake[MAX_LEN];
int snake_len;
struct pos direction;

void change_dir()
{
	struct pos next;
	next.x = 0;
	next.y = 0;
	if (status[KEY_UP])
	{
		next.y = -1;
		goto check;
	}
	if (status[KEY_DOWN])
	{
		next.y = 1;
		goto check;
	}
	if (status[KEY_LEFT])
	{
		next.x = -1;
		goto check;
	}
	if (status[KEY_RIGHT])
	{
		next.x = 1;
		goto check;
	}
check:
	if (next.x == 0 && next.y == 0)
		return;
	if ((next.x + direction.x == 0) && (next.y + direction.y == 0))
		return;

	direction = next;	
}

void move_snake()
{
	int i = snake_len;

	while(i-- > 1)
		snake[i] = snake[i - 1];

	snake[0] = snake[1];
	snake[0].x = (SCREEN_WIDTH + snake[0].x + direction.x) % SCREEN_WIDTH;
	snake[0].y = (SCREEN_HEIGHT + snake[0].y + direction.y) % SCREEN_HEIGHT;
}

void draw_snake()
{
	int i = 0;
	
	memset((uint8*)buf, ' ', sizeof(buf));
	for (i = 0; i < snake_len; ++i)
		buf[snake[i].x + snake[i].y * SCREEN_WIDTH] = symbol;
}

void enlarge_snake()
{
	if (iterations % 16 != 0)
		return;
	if (snake_len == MAX_LEN)
		return;
	snake[snake_len] = snake[snake_len - 1];
	++snake_len;
}

int main()
{
	int screen;
	int keyboard;
	int i;
	int tick = 50;
	uint64 start, stop;

	snake[0].x = 20;
	snake[0].y = 12;
	snake_len = 5;
	direction.x = 1;
	direction.y = 0;

	register_process("snake");

	for (i = 1; i < snake_len; ++i)
	{
		snake[i].x = snake[i - 1].x - direction.x;
		snake[i].y = snake[i - 1].y - direction.y;
	}

	while((screen = connect("sys.drivers.fullscreen")) == -1)
		wait(100);

	keyboard = select();
	read(keyboard, (uint8*)status, sizeof(status));

	while(1)
	{
		start = time();
		change_dir();
		enlarge_snake();
		move_snake();
		draw_snake();
		write(screen, (uint8*)buf, sizeof(buf));

		if(peek(keyboard) > 0)
			read(keyboard, (uint8*)status, sizeof(status));

		stop = time();
		wait(tick - stop + start);
		++iterations;
	}

	return 0;
}
