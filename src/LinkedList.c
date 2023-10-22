#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "LinkedList.h"

ll_t *LinkedListCreate(void *p)
{
	ll_t *l = calloc(1, sizeof(ll_t));

	if (l)
	{
		l->p = p;
		l->next = NULL;
	}

	return l;
}

void LinkedListDestroy(ll_t *l)
{
	ll_t* t = NULL;

	do
	{
		t = l->next;
		free(l);
		l = t;
	}
	while (l);
}

void LinkedListDestroyAndFree(ll_t *l)
{
	ll_t *t = l->next;

	do
	{
		t = l->next;
		free(l->p);
		free(l);
		l = t;
	}
	while (l);
}

ll_t* LinkedListAppend(ll_t *l, void *p)
{
	while (l->next != NULL)
	{
		l = l->next;
	}

	l->next = LinkedListCreate(p);

	return l->next;
}

ll_t* LinkedListPrepend(ll_t *l, void *p)
{
	ll_t* h = LinkedListCreate(p);

	if (h)
	{
		h->next = l;
	}

	return h;
}

void LinkedListForEach(ll_t *l, void (*cb)(void *p))
{
	while (l)
	{
		cb(l->p);
		l = l->next;
	}
}

bool LinkedListSome(ll_t *l, void* param, bool (*cb)(void *, void*))
{
	return LinkedListFirst(l, param, cb) != NULL;
}

void* LinkedListFirst(ll_t* l, void* param, bool (*cb)(void *, void*))
{
	// printf("LinkedListFirst(%p, '%s', <fpointer>)\n", (void*)l, (char*)param);
	while (l)
	{
		// printf("Calling cb(%p, '%s')\n", (void*)l->p, (char*)param);
		if (cb(l->p, param))
		{
			return l->p;
		}
		l = l->next;
	}
	return NULL;
}
