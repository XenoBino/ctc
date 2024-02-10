#include "hashmap.h"
#include <stdlib.h>
#include "oom.h"
#include "die.h"
#include <xxh3.h>
#include <string.h>

#define HM_DEFAULT_CAP 4096

Hashmap *hashmap_create(size_t bucket_capacity, HashType hash) {
	if (!hash) hash = CTC_HM_HASH_XXH3;
	if (hash != CTC_HM_HASH_XXH3) die("Parameter 'hash' is not valid!\n");

	Hashmap *map = malloc(sizeof(*map));
	if (!map) oom();
	map->capacity = bucket_capacity;
	if (!map->capacity) map->capacity = HM_DEFAULT_CAP;
	map->bucket = calloc(map->capacity, sizeof(HashNode));
	if (!map->bucket) oom();

	return map;
}

HashNode *hashmap_put(Hashmap *map, const void *key, size_t keylen, void *value) {
	uint64_t hash = hashmap_hash(map, key, keylen);
	size_t index = hash % map->capacity;
	HashNode **t = &map->bucket[index];
	if (!(*t)) {
		HashNode *node = malloc(sizeof(*node));
		if (!node) oom();
		node->key = malloc(keylen);
		if (!node->key) oom();
		memcpy(node->key, key, keylen);
		node->keylen = keylen;
		node->value = value;
		node->next = NULL;
		*t = node;
		return node;
	}

	HashNode *prev = NULL;
	HashNode *curr = *t;

	while (curr) {
		if (!memcmp(curr->key, key, keylen)) {
			curr->value = value;
			return curr;
		}

		prev = curr;
		curr = curr->next;
	}

	HashNode *node = malloc(sizeof(*node));
	if (!node) oom();
	node->key = malloc(keylen);
	if (!node->key) oom();
	memcpy(node->key, key, keylen);
	node->keylen = keylen;
	node->value = value;
	node->next = NULL;
	prev->next = node;
	return node;
}

HashNode *hashmap_get(Hashmap *map, const void *key, size_t keylen) {
	uint64_t hash = hashmap_hash(map, key, keylen);
	size_t index = hash % map->capacity;

	HashNode *curr = map->bucket[index];
	while (curr) {
		if (!memcmp(curr->key, key, keylen)) return curr;
		curr = curr->next;
	}

	return NULL;
}

uint64_t hashmap_hash(Hashmap *map, const void *key, size_t keylen) {
	return XXH64(key, keylen, 0);
}

void hashmap_destroy(Hashmap *map) {
	HashNode **iter = map->bucket;
	HashNode **end = map->bucket + map->capacity;
	while (iter < end) {
		HashNode *curr = *iter;
		while (curr) {
			HashNode *next = curr->next;
			free(curr->key);
			free(curr);
			curr = next;
		}
		iter++;
	}

	free(map->bucket);
	free(map);
}
