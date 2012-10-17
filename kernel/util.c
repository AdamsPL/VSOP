#include "util.h"

int int2str(char *str, unsigned int n, int base) {
	static const char digits[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'};
	char buf[32];
	char *ptr = buf;
	int count = 0;
	int res;

	buf[count++] = '\0';

	do{
		buf[count++] = digits[n % base];
		n /= base;
	}while(n > 0);

	res = count;
	ptr = buf + count - 1;

	while((*str++ = *ptr--))
		;
	return res;
}

char *kprintf(char *str, const char *format, ...) {
	const char *c = format;
	unsigned int *stack = (uint32*)&format + 1;
	char *buf = str;
	char *tmp;
	while(*c) {
		if (*c == '%') {
			++c;
			switch(*c) {
				case 'c':
					*str++ = (char)*stack;
					break;
				case 'i':
					str += int2str(str, *stack, 10) - 1;
					break;
				case 'x':
					*str++ = '0';
					*str++ = 'x';
					str += int2str(str, *stack, 16) - 1;
					break;
				case 'b':
					str += int2str(str, *stack, 2) - 1;
					break;
				case 's':
					tmp = (char*) *stack;
					while((*str++ = *tmp++));
					str--;
			}
			++stack;
		} else {
			*str++ = *c;
		}
		++c;
	}
	*str++ = '\0';
	return buf;
}

int kstrcmp(char *str1, char *str2){
	while((*str1 == *str2) && *str1){
		str1++;
		str2++;
	}
	if (*str1 == *str2)
		return 0;
	if (*str1 < *str2)
		return -1;
	else
		return 1;
}

#define KMEMSET_IMPL(name, type) void *name(type *ptr, type value, uint32 count) \
{                       \
	type *tmp = ptr;    \
	while(count--)      \
		*tmp++ = value; \
	return ptr;         \
}

KMEMSET_IMPL(kmemset, uint8)
KMEMSET_IMPL(kmemset16, uint16)
KMEMSET_IMPL(kmemset32, uint32)

#undef KMEMSET_IMPL

void *kmemcpy(uint8 *destination, const uint8 *source, uint32 count)
{
	uint8 *end = (uint8*)source + count;
	while(source < end) {
		*((char*)destination++) = *((char*)source++);
	}
	return destination;
}

void kstrncpy(uint8 *destination, const uint8 *source, uint32 count)
{
	while ((*destination++ = *source++) && count)
		--count;
}

int truncate(int value, int min, int max)
{
	if (value > max)
		return max;
	if (value < min)
		return min;
	return value;
}
