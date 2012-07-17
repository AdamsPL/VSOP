#ifndef _SYSCALL
#define _SYSCALL

#include "multitasking.h"
#include "system_calls.h"

void syscall(struct thread_state *state);

#endif
