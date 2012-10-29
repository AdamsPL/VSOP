#ifndef _SCHEDULER_H
#define _SCHEDULER_H

#include "config.h"
#include "thread.h"
#include "process.h"

void scheduling_init();
void sched_thread_ready(struct thread *thread);
void sched_thread_sleep(uint64 ticks);
void sched_thread_wait_for_msg();
void sched_yield(void);
struct thread *sched_cur_thread(void);
struct process *sched_cur_proc(void);
void sched_start_timer();
uint8 sched_tick(struct thread_state *state);
void sched_idle_loop(void);

#endif 
