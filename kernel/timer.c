#include "timer.h"
#include "interrupts.h"
#include "ports.h"
#include "memory.h"
#include "scheduler.h"
#include "screen.h"

#define TIMER_HZ 1024

#define DAY_TO_H 24
#define H_TO_MIN 60
#define MIN_TO_SEC 60
#define SEC_TO_MS 1000
#define MS_TO_US 1000

static uint64 ticks = 0;
static struct thread *next_thread = 0;
static lock_t lock;

static void _wait_list_add(struct thread *thread)
{
	struct thread **ptr = &next_thread;
	section_enter(&lock);

	while (*ptr && (*ptr)->wait_time <= thread->wait_time)
		
		ptr = &((*ptr)->next);
	if (!*ptr)
		goto exit;

	thread->next = (*ptr);

exit:
	*ptr = thread;
	section_leave(&lock);
}

static uint8 _tick(struct thread_state *regs)
{
	struct thread *tmp;
	++ticks;

	section_enter(&lock);

	while(next_thread && (next_thread->wait_time <= ticks))
	{
		tmp = next_thread->next;
		next_thread->wait_time = 0;
		next_thread->next = 0;
		sched_thread_ready(next_thread);
		next_thread = tmp;
	}
	section_leave(&lock);

	return INT_OK;
}

void timer_init()
{
	ticks = 0;
	next_thread = 0;

	port_write(0x70, 0x0B);
	char prev = port_read_8(0x71);
	port_write(0x70, 0x0B);
	port_write(0x71, prev | 0x40);

	interrupts_register_handler(INT_RTC, _tick);
}

uint64 _time_to_ticks(struct time_t time)
{
	long long cur_ticks = 0;
	
	cur_ticks += time.milisec * TIMER_HZ / SEC_TO_MS;
	cur_ticks += time.sec * TIMER_HZ;
	cur_ticks += time.minutes * TIMER_HZ * MIN_TO_SEC;
	cur_ticks += time.hours * TIMER_HZ * MIN_TO_SEC * H_TO_MIN;
	cur_ticks += time.days * TIMER_HZ * MIN_TO_SEC * H_TO_MIN * DAY_TO_H;

	return cur_ticks;
}

//TODO: debug this linking udivdi3 error
static struct time_t _ticks_to_time(uint32 cur_ticks)
{
	struct time_t uptime;

	uptime.days = (cur_ticks) / (TIMER_HZ * MIN_TO_SEC * H_TO_MIN * DAY_TO_H);
	cur_ticks %= (TIMER_HZ * MIN_TO_SEC * H_TO_MIN * DAY_TO_H);

	uptime.hours = (cur_ticks) / (TIMER_HZ * MIN_TO_SEC * H_TO_MIN);
	cur_ticks %= (TIMER_HZ * MIN_TO_SEC * H_TO_MIN);

	uptime.minutes = (cur_ticks) / (TIMER_HZ * MIN_TO_SEC);
	cur_ticks %= (TIMER_HZ * MIN_TO_SEC);

	uptime.sec = (cur_ticks) / (TIMER_HZ);
	cur_ticks %= (TIMER_HZ);

	uptime.milisec = (cur_ticks * SEC_TO_MS) / (TIMER_HZ);

	return uptime;
}

struct time_t timer_uptime()
{
	return _ticks_to_time(ticks);
}

void timer_manage_thread(struct thread *thread)
{
	thread->wait_time += ticks;
	thread->next = 0;
	_wait_list_add(thread);
}

void timer_active_wait(uint32 wtime)
{
	wtime += ticks;
	while(ticks < wtime);
}
