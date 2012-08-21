#ifndef _LOCKS_H
#define _LOCKS_H

#include "config.h"

void section_enter(lock_t lock);
void section_leave(lock_t lock);

#endif
