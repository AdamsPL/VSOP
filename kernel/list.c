#include "list.h"
#include "memory.h"
#include "util.h"
#include "screen.h"

struct list_elem *list_elem_new(void *target)
{
	struct list_elem *result = NEW(struct list_elem);
	result->target = target;
	return result;
}

void list_push(struct list *this, void *target)
{
	struct list_elem *elem = list_elem_new(target);

	section_enter(&this->lock);

	if (!this->tail)
	{
		this->head = elem;
		this->tail = elem;
		goto cleanup;
	}
	this->tail->next = elem;
	this->tail = elem;

cleanup:
	section_leave(&this->lock);
}

void *list_pop(struct list *this)
{
	struct list_elem *elem;
	void *result = 0;

	section_enter(&this->lock);

	elem = this->head;

	if (this->head == this->tail)
	{
		this->head = 0;
		this->tail = 0;
	}
	else
		this->head = this->head->next;

	if (elem)
	{
		result = elem->target;
		DELETE(elem);
	}

	section_leave(&this->lock);
	return result;
}

void list_print(struct list *this)
{
	char buf[128];
	struct list_elem *ptr = this->head;

	screen_putstr(kprintf(buf, "%x: ", this));
	while(ptr)
	{
		screen_putstr(kprintf(buf, "%x->", ptr->target));
		ptr = ptr->next;
	}
	screen_putstr(kprintf(buf, "\n", this));
}

void *list_peek(struct list *this)
{
	struct list_elem *ptr = this->head;
	if (!ptr)
		return ptr;
	return ptr->target;
}
