#include "list.h"
#include "memory.h"
#include "util.h"
#include "screen.h"

static void list_lock(struct list *this)
{
	section_enter(&this->lock);
}

static void list_unlock(struct list *this)
{
	section_leave(&this->lock);
}

static struct list_elem *list_elem_new(struct list *this, void *target)
{
	struct list_elem *result;

	if (this->free)
	{
		result = this->free;
		this->free = this->free->next;
	}
	else
		result = NEW(struct list_elem);

	result->target = target;
	result->next = 0;
	return result;
}

static void list_elem_delete(struct list *this, struct list_elem *list_elem)
{
	list_elem->next = this->free;
	list_elem->target = 0;
	this->free = list_elem;
}

void list_push(struct list *this, void *target)
{
	struct list_elem *elem;
   
	list_lock(this);

	elem = list_elem_new(this, target);

	if (!this->tail)
	{
		this->head = elem;
		this->tail = elem;
		goto cleanup;
	}
	this->tail->next = elem;
	this->tail = elem;

cleanup:
	++this->size;
	list_unlock(this);
}

void *list_pop(struct list *this)
{
	struct list_elem *elem;
	void *result = 0;

	if (this->size == 0)
		return 0;
	
	list_lock(this);

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
		list_elem_delete(this, elem);
	}

	if (result)
		--this->size;
	list_unlock(this);
	return result;
}

void list_print(struct list *this)
{
	char buf[128];
	struct list_elem *ptr = this->head;

	screen_putstr(kprintf(buf, "%x(elem:%i): ", this, this->size));
	while(ptr)
	{
		screen_putstr(kprintf(buf, "%x->", ptr->target));
		ptr = ptr->next;
	}
	screen_putstr(kprintf(buf, "\n", this));
}

int list_size(struct list *this)
{
	return this->size;
}

void list_prealloc(struct list *this, int count)
{
	while(count-- > 0)
	{
		struct list_elem *elem = NEW(struct list_elem);
		list_elem_delete(this, elem);
	}
}

