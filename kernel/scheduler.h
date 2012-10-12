#ifndef _SCHEDULER_H
#define _SCHEDULER_H

#include "config.h"
#include "thread.h"

void sched_init();
void sched_thread_ready(struct thread *thread);
void sched_thread_sleep(uint64 ticks);
int sched_thread_select_msg();
pid_t sched_cur_proc();
void sched_yield(void);
struct thread *sched_cur_thread(void);
void sched_start_timer();
uint8 sched_tick(struct thread_state *state);

#endif 
