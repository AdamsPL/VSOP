#ifndef _SYSCALL
#define _SYSCALL

#include "config.h"
#include "system_calls.h"
#include "thread.h"

uint8 syscall(struct thread_state *state);
void syscalls_init(void);

#endif
