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

static const int SCREEN_HEIGHT = 25;
static const int SCREEN_WIDTH = 80;

static uint8 cur_style = (BLACK << 4 | LIGHTGREY);
unsigned short *videomem;

static uint8 cur_x = 0;
static uint8 cur_y = 0;

static void mv_cur(char c) {
	switch(c){
		case '\r':
			cur_x = 0;
			break;
		case '\n':
			cur_x = 0;
			++cur_y;
			break;
		default:
			++cur_x;
	}
	if (cur_x > SCREEN_WIDTH) {
		cur_y += cur_x / SCREEN_WIDTH;
		cur_x %= SCREEN_WIDTH;
	}
}

static uint8 screen_getstyle(enum ScreenColor bg, enum ScreenColor fg)
{
	uint8 ret = bg;
	ret <<= 4;
	ret |= fg;
	return ret;
}

static void clear_line(uint8 lineno)
{
	uint8 c = cur_style >> 4;
	uint16 blank;

	blank = screen_getstyle(c, c) << 8;
	blank |= ' ';
	memset16(videomem + SCREEN_WIDTH*(SCREEN_HEIGHT-1), blank, SCREEN_WIDTH);
}

static void move_screen()
{
	memcpy((uint8*)videomem, (uint8*)(videomem + SCREEN_WIDTH), sizeof(uint16)*SCREEN_WIDTH*(SCREEN_HEIGHT-1));
}

static void screen_putchar(char c, uint8 style)
{
	uint16 value;
	if (cur_y >= SCREEN_HEIGHT) {
		move_screen();
		clear_line(SCREEN_HEIGHT-1);
		cur_y = SCREEN_HEIGHT-1;
	}
	value = style;
	value <<= 8;
	value |= c;
	if (c >= ' ')
		*(videomem + cur_y * SCREEN_WIDTH + cur_x) = value;
	mv_cur(c);
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

void screen_putstr(char *c)
{
	while(*c)
		screen_putchar(*c++, cur_style);
}

int main()
{
	char out[32];
	char buf[512];
	int stream = -1;
	int len = 0;
	int i = 0;
	int tmp;

	videomem = (unsigned short*)mmap((void*)0xB8000);
	cur_x = 0;
	cur_y = 0;

	proc_register("sys.drivers.screen", -1);
	screen_clear();
	while(1){
		if ((len = receive(&stream, buf, 32))){
			for (i = 0; i < len; ++i){
				tmp = buf[i] & 0xFF;
				printf(out, "%c", tmp);
				screen_putstr(out);
			}
		}
	}
	return 0;
}
