#include "screen.h"
#include "util.h"
#include "config.h"
#include "locks.h"

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
unsigned short *videomem = (unsigned short*)0xB8000;

static uint8 cur_x = 0;
static uint8 cur_y = 0;

static lock_t screen_lock = 0;

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
	kmemset16(videomem + SCREEN_WIDTH*(SCREEN_HEIGHT-1), blank, SCREEN_WIDTH);
}

static void move_screen()
{
	kmemcpy((uint8*)videomem, (uint8*)(videomem + SCREEN_WIDTH), sizeof(uint16)*SCREEN_WIDTH*(SCREEN_HEIGHT-1));
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

	section_enter(&screen_lock);
	blank |= ' ';
	kmemset16(videomem, blank, SCREEN_HEIGHT * SCREEN_WIDTH);
	cur_x = 0;
	cur_y = 0;
	section_leave(&screen_lock);
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
	/*
	section_enter(&screen_lock);
	*/
	while(*c)
		screen_putchar(*c++, cur_style);
	/*
	section_leave(&screen_lock);
	*/
}
