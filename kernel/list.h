#ifndef _LIST_H
#define _LIST_H

#include "locks.h"

struct list_elem
{
	void *target;
	struct list_elem *next;
};

struct list
{
	int size;
	struct list_elem *head;
	struct list_elem *tail;
};

struct list_elem *list_elem_new(void *target);

void list_push(struct list *this, void *target);
void *list_pop(struct list *this);
void list_print(struct list *this);
int list_size(struct list *this);

#endif
