#include "timer.h"
#include "interrupts.h"
#include "ports.h"
#include "memory.h"
#include "scheduler.h"
#include "screen.h"
#include "locks.h"

#define TIMER_HZ 1024

#define DAY_TO_H 24
#define H_TO_MIN 60
#define MIN_TO_SEC 60
#define SEC_TO_MS 1000
#define MS_TO_US 1000

struct wait_list_elem
{
	struct thread *thread;
	struct wait_list_elem *next;
	uint64 wait_time;
};

static uint64 ticks = 0;
static struct wait_list_elem *next_elem = 0;
static lock_t lock;


static void _wait_list_add(struct thread *thread)
{
	struct wait_list_elem **ptr = &next_elem;
	struct wait_list_elem *elem = NEW(struct wait_list_elem);

	elem->wait_time = thread->wait_time;
	elem->thread = thread;

	section_enter(&lock);

	while (*ptr && (*ptr)->wait_time <= elem->wait_time)
		ptr = &((*ptr)->next);
	if (!*ptr)
		goto exit;

	elem->next = (*ptr);

exit:
	*ptr = elem;
	section_leave(&lock);
}

static uint8 _tick(struct thread_state *regs)
{
	/*
	char buf[128];
	*/
	struct wait_list_elem *tmp;
	++ticks;

	section_enter(&lock);

	while(next_elem && (next_elem->wait_time <= ticks))
	{
		tmp = next_elem->next;
		next_elem->wait_time = 0;
		next_elem->next = 0;
		next_elem->thread->wait_time = 0;
		/*
		screen_putstr(kprintf(buf, "timer rdy\n"));
		*/
		sched_thread_ready(next_elem->thread);
		DELETE(next_elem);
		next_elem = tmp;
	}
	section_leave(&lock);

	return INT_OK;
}

void timer_init()
{
	char prev;

	ticks = 0;
	next_elem = 0;

	port_write(0x70, 0x0B);
	prev = port_read_8(0x71);
	port_write(0x70, 0x0B);
	port_write(0x71, prev | 0x40);

	interrupts_register_handler(INT_RTC, _tick);
}

uint64 _time_to_ticks(struct time_t time)
{
	uint64 cur_ticks = 0;
	
	cur_ticks += time.milisec * TIMER_HZ / SEC_TO_MS;
	cur_ticks += time.sec * TIMER_HZ;
	cur_ticks += time.minutes * TIMER_HZ * MIN_TO_SEC;
	cur_ticks += time.hours * TIMER_HZ * MIN_TO_SEC * H_TO_MIN;
	cur_ticks += time.days * TIMER_HZ * MIN_TO_SEC * H_TO_MIN * DAY_TO_H;

	return cur_ticks;
}

/*TODO: debug this linking udivdi3 error*/
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
	_wait_list_add(thread);
}

void timer_active_wait(uint32 wtime)
{
	wtime += ticks;
	while(ticks < wtime);
}

uint64 timer_get_ticks(void)
{
	return ticks;
}
