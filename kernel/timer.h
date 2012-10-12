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
void timer_manage_thread(struct thread *thread);
void timer_active_wait(uint32 wtime);
uint64 timer_get_ticks(void);

#endif
