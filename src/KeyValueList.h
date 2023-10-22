#ifndef KEY_VALUE_LIST_H_
#define KEY_VALUE_LIST_H_

#include <stdbool.h>

#include "LinkedList.h"

typedef struct {
	char *k;
	char *v;
} kv_t;

typedef ll_t kvl_t;

kvl_t *KeyValueListCreate(const char* key, const char *value);

void KeyValueListDestroy(kvl_t *l);

bool KeyValueListExists(kvl_t *l, const char *key);

char *KeyValueListGet(kvl_t *l, const char *key);

void KeyValueListSet(kvl_t *l, const char *key, const char *value);

#endif

