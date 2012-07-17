#ifndef _LOCKS_H
#define _LOCKS_H

#include "config.h"

void section_enter(uint32 *lock);
void section_leave(uint32 *lock);

#endif
