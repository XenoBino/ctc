#ifndef XENO_CTC_HASHMAP_H_
#define XENO_CTC_HASHMAP_H_

#include <stddef.h>
#include <stdint.h>

typedef enum {
	CTC_HM_HASH_DEFAULT = 0,
	CTC_HM_HASH_XXH3
} HashType;

typedef struct _HashNode HashNode;

struct _HashNode {
	HashNode *next;
	void *value;
	void *key;
	size_t keylen;
};

typedef struct {
	HashNode **bucket;
	size_t capacity;
} Hashmap;

Hashmap *hashmap_create(size_t bucket_size, HashType hash);
HashNode *hashmap_put(Hashmap *map, const void *key, size_t keylen, void *value);
HashNode *hashmap_get(Hashmap *map, const void *key, size_t keylen);
uint64_t hashmap_hash(Hashmap *map, const void *key, size_t keylen);
void hashmap_destroy(Hashmap *map);

#endif
