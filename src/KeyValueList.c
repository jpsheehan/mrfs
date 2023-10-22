#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "LinkedList.h"
#include "KeyValueList.h"

kv_t* _KeyValueCreate(const char* key, const char* value);

void _KeyValueDestroy(kv_t* kv);

bool _KeyValueListExistsCallback(void *key, void *kv);

void _KeyValueListFreeCallback(void *kv);

kvl_t* KeyValueListCreate(const char* key, const char *value)
{
	kvl_t *l = NULL;
	kv_t *kv = _KeyValueCreate(key, value);

	if (kv)
	{
		l = LinkedListCreate(kv);
		if (l == NULL)
		{
			_KeyValueDestroy(kv);
			kv = NULL;
		}
		else
		{
			// printf("Created LinkedList\n");
			// printf("It has '%s' = '%s'\n", kv->k, kv->v);
		}
	}

	return l;
}


void KeyValueListDestroy(kvl_t *l)
{
	LinkedListForEach((ll_t*)l, _KeyValueListFreeCallback);
	LinkedListDestroy((ll_t*)l);
}

bool KeyValueListExists(kvl_t *l, const char *key)
{
	return LinkedListSome((ll_t*)l, (char*)key, _KeyValueListExistsCallback);
}

char *KeyValueListGet(kvl_t *l, const char *key)
{
	kv_t* kv = NULL;

	kv = LinkedListFirst((ll_t*)l, (char*)key, _KeyValueListExistsCallback);

	if (kv)
	{
		return kv->v;
	}

	return NULL;
}

void KeyValueListSet(kvl_t *l, const char *key, const char *value)
{
	kv_t *kv = NULL;

	// printf("KeyValueListSet(%p, '%s', '%s')\n", (void*)l, key, value);
	kv = LinkedListFirst((ll_t*)l, (char*)key, _KeyValueListExistsCallback);

	if (kv)
	{
		free(kv->v);
		kv->v = (char*)strdup(value);
	}
	else
	{
		kv = _KeyValueCreate(key, value);
		LinkedListAppend((ll_t*)l, kv);
	}
}

kv_t* _KeyValueCreate(const char* key, const char* value)
{
	kv_t* kv = NULL;
	kv = calloc(1, sizeof(kv_t));
	if (kv)
	{
		kv->k = (char*)strdup(key);
		if (kv->k) {
			
			kv->v = (char*)strdup(value);
			if (kv->v == NULL)
			{
				free(kv->k);
				free(kv);
				kv = NULL;
			}
		}
		else
		{
			free(kv);
			kv = NULL;
		}
	}

	return kv;
}

void _KeyValueDestroy(kv_t* kv)
{
	free(kv->v);
	free(kv->k);
	free(kv);
}

void _KeyValueListFreeCallback(void *kv)
{
	_KeyValueDestroy(kv);
}

bool _KeyValueListExistsCallback(void *kv, void* key)
{
	return (strcmp((char*)key, ((kv_t*)kv)->k) == 0);
}

