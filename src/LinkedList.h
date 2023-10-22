#ifndef LINKED_LIST_H_
#define LINKED_LIST_H_

#include <stdbool.h>

typedef struct _ll_s {
	void *p;
	struct _ll_s *next;
} ll_t;

ll_t *LinkedListCreate(void *p);

void LinkedListDestroy(ll_t *l);
void LinkedListDestroyAndFree(ll_t *l);

ll_t* LinkedListAppend(ll_t *l, void *p);
ll_t* LinkedListPrepend(ll_t *l, void *p);

void LinkedListForEach(ll_t *l, void (*cb)(void *p));
bool LinkedListSome(ll_t *l, void* param, bool (*cb)(void *p, void* param));
void* LinkedListFirst(ll_t* l, void* param, bool (*cb)(void *p, void* param));

#endif

