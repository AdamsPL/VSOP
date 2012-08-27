#ifndef _TIMER_H
#define _TIMER_H

#include "config.h"
#include "thread.h"

struct time_t
{
	uint16 milisec;
	uint8 sec;
	uint8 minutes;
	uint8 hours;
	uint32 days;
};

void timer_init();
struct time_t timer_uptime();

#endif