#include <stdlib.h>

enum scancode
{
	ERR,
	ESC,
	D_1,
	D_2,
	D_3,
	D_4,
	D_5,
	D_6,
	D_7,
	D_8,
	D_9,
	D_0,
	D_MINUS,
	D_PLUS,
	D_BACKSPACE,
	D_TAB,
	D_Q,
	D_W,
	D_E,
	D_R,
	D_T,
	D_Y,
	D_U,
	D_I,
	D_O,
	D_P,
	D_LSB,
	D_RSB,
	D_ENTER,
	D_LCTRL,
	D_A,
	D_S,
	D_D,
	D_F,
	D_G,
	D_H,
	D_J,
	D_K,
	D_L,
	D_COLON,
	D_APO,
	D_TILDA,
	D_LSHIFT,
	D_BACKSLASH,
	D_Z,
	D_X,
	D_C,
	D_V,
	D_B,
	D_N,
	D_M,
	D_LESS,
	D_GTHEN,
	D_SLASH,
	D_RSHIFT,
	D_KEYPAD,
	D_LALT,
	D_SPACE,
	D_CAPS,
	D_F1,
	D_F2,
	D_F3,
	D_F4,
	D_F5,
	D_F6,
	D_F7,
	D_F8,
	D_F9,
	D_F10,
	D_NUMLOCK,
	D_SCROLL,
	D_KEYPAD7,
	D_KEYPAD8,
	D_KEYPAD9,
	D_KEYPADMINUS,
	D_KEYPAD4,
	D_KEYPAD5,
	D_KEYPAD6,
	D_KEYPADPLUS,
	D_KEYPADEND,
	D_KEYPAD2,
	D_KEYPAD3,
	D_KEYPAD0,
	D_KEYPADDOT,
	D_ALTSYS,
	D_FN,
	TOTAL_KEYS
};

char get_ascii_char(char scancode)
{
	if (scancode >= 128)
		return 0;
	switch(scancode){
		case D_A: return 'a';
		case D_S: return 's';
		case D_D: return 'd';
		case D_F: return 'f';
		case D_G: return 'g';
		case D_H: return 'h';
		case D_J: return 'j';
		case D_K: return 'k';
		case D_L: return 'l';
		case D_Q: return 'q';
		case D_W: return 'w';
		case D_E: return 'e';
		case D_R: return 'r';
		case D_T: return 't';
		case D_Y: return 'y';
		case D_U: return 'u';
		case D_I: return 'i';
		case D_O: return 'o';
		case D_P: return 'p';
		case D_Z: return 'z';
		case D_X: return 'x';
		case D_C: return 'c';
		case D_V: return 'v';
		case D_B: return 'b';
		case D_N: return 'n';
		case D_M: return 'm';
		case D_1: return '1';
		case D_2: return '2';
		case D_3: return '3';
		case D_4: return '4';
		case D_5: return '5';
		case D_6: return '6';
		case D_7: return '7';
		case D_8: return '8';
		case D_9: return '9';
		case D_0: return '0';
		case D_SPACE: return ' ';
		case D_ENTER: return '\n';
		default: return 0;
	}
}

int main()
{
	int pid = -1;
	char buf[128];
	int stream = -1;
	int tmp;
	
	proc_register("sys.drivers.keyboard", 217);

	while(pid == -1){
		pid = proc_query("sys.drivers.screen");
	}

	stream = connect(pid);
	while(1){
		while(!receive(&tmp, buf, 32)); 
		buf[0] = get_ascii_char(port_read8(0x60));
		if (buf[0])
			while(!send(stream, buf, 1)); 
	}
	return 0;
}
