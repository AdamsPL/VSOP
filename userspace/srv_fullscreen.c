#include <stdlib.h>

enum ScreenColor{
	BLACK = 0,
	BLUE,
	GREEN,
	CYAN,
	RED,
	MAGENTA,
	BROWN,
	LIGHTGREY,
	DARKGRAY,
	LIGHTBLUE,
	LIGHTGREEN,
	LIGHTCYAN,
	LIGHTRED,
	LIGHTMAGENTA,
	LIGHTBROWN,
	WHITE
};

#define SCREEN_HEIGHT 25
#define SCREEN_WIDTH 80

unsigned short *videomem = (unsigned short *)0xA0000000;

static uint8 cur_style = (BLACK << 4 | LIGHTGREY);

static uint8 cur_x = 0;
static uint8 cur_y = 0;

static uint8 screen_getstyle(enum ScreenColor bg, enum ScreenColor fg)
{
	uint8 ret = bg;
	ret <<= 4;
	ret |= fg;
	return ret;
}

void screen_clear()
{
	uint8 c = cur_style >> 4;
	uint16 blank = screen_getstyle(c, c) << 8;
	blank |= ' ';
	memset16(videomem, blank, SCREEN_HEIGHT * SCREEN_WIDTH);
}

void screen_set_bg(uint8 color)
{
	cur_style &= 0x0f;
	cur_style |= color << 4;
}

void screen_set_fg(uint8 color)
{
	cur_style &= 0xf0;
	cur_style |= color;
}

static void screen_display(char *buf)
{
	uint16 value;
	int i;
	for (i = 0; i < SCREEN_WIDTH * SCREEN_HEIGHT * sizeof(char); ++i)
	{
		value = cur_style;
		value <<= 8;
		value |= buf[i];
		videomem[i] = value;
	}
}

int main()
{
	char msg[SCREEN_HEIGHT * SCREEN_WIDTH];
	int descr;

	mmap(videomem, (void*)0xB8000);
	cur_x = 0;
	cur_y = 0;

	register_process("sys.drivers.fullscreen");
	screen_set_bg(BLACK);
	screen_set_fg(WHITE);
	screen_clear();
	while(1)
	{
		descr = select();
		read(descr, (uint8*)msg, SCREEN_HEIGHT * SCREEN_WIDTH);
		screen_display(msg);
	}
	
	return 0;
}
