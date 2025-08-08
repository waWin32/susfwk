// hashmap.c
//
#include "coreframe.h"
#include "include/susfwk/core.h"
#include "include/susfwk/memory.h"
#include "include/susfwk/crypto.h"
#include "include/susfwk/linkedList.h"
#include "include/susfwk/hashmap.h"

// Create a hash table
SUS_HASHMAP SUSAPI susNewMapEx(_In_ DWORD initCount)
{
	SUS_PRINTDL("Creating a new hash table");
	SUS_HASHMAP map = sus_malloc(sizeof(SUS_HASHMAP_STRUCT) + (initCount ? initCount : SUS_HASHMAP_INIT_COUNT) * sizeof(SUS_LIST));
	if (!map) return NULL;
	map->capacity = initCount ? initCount : SUS_HASHMAP_INIT_COUNT;
	map->count = 0;
	for (DWORD i = 0; i < map->capacity; i++) map->buckets[i] = susNewList();
	return map;
}
// Add a new key-value pair to the hash table
SUS_NODE SUSAPI susMapAdd(_Inout_ SUS_LPHASHMAP lpMap, _In_ LPCSTR key, _In_ SUS_DATAVIEW value)
{
	SUS_ASSERT(!susMapGet(*lpMap, key));
	SUS_HASHMAP map = *lpMap;
	SUS_LPLIST bucket = &map->buckets[susMapGetIndex(map, key)];
	SUS_NODE node = susListPushBack(bucket, sizeof(SUS_MAPENTRY_STRUCT) + value.size);
	if (!node) return NULL;
	SUS_MAPENTRY keyValue = (SUS_MAPENTRY)node->data;
	sus_memcpy(keyValue->value, value.data, value.size);
	keyValue->size = value.size;
	keyValue->key = sus_malloc(lstrlenA(key) + sizeof(CHAR));
	lstrcpyA(keyValue->key, key);
	map->count++;
	susMapOptimization(lpMap);
	return node;
}
// Get an item by key
SUS_NODE SUSAPI susMapGet(_In_ SUS_HASHMAP map, _In_ LPCSTR key)
{
	SUS_LIST bucket = map->buckets[susMapGetIndex(map, key)];
	susListForeach(node, bucket) {
		SUS_MAPENTRY keyValue = (SUS_MAPENTRY)node->data;
		if (lstrcmp(keyValue->key, key) == 0) {
			return node;
		}
	}
	return NULL;
}
// Change the size of the hash table
SUS_HASHMAP SUSAPI susMapCopy(_In_ SUS_HASHMAP source, _In_ DWORD newCount)
{
	SUS_HASHMAP newHt = susNewMapEx(newCount);
	if (!newHt) return NULL;
	for (DWORD i = 0; i < source->capacity; i++) {
		SUS_LIST bucket = source->buckets[i];
		susListForeach(node, bucket) {
			SUS_MAPENTRY kv = (SUS_MAPENTRY)node->data;
			susMapAdd(&newHt, kv->key, (SUS_DATAVIEW) { .data = kv->value, .size = kv->size });
		}
	}
	return newHt;
}
// Optimizing a hash table by resizing it for improved performance
VOID SUSAPI susMapOptimization(_Inout_ SUS_LPHASHMAP lpMap)
{
	SUS_HASHMAP map = *lpMap;
	if (map->count > map->capacity * SUS_HASHMAP_RATIO) {
		susMapResize(lpMap, map->capacity * SUS_HASHMAP_GROWTH_FACTOR);
	}
	else if (map->count < map->capacity * (1 - SUS_HASHMAP_RATIO) && map->capacity > SUS_HASHMAP_INIT_COUNT * SUS_HASHMAP_GROWTH_FACTOR) {
		susMapResize(lpMap, map->count * SUS_HASHMAP_GROWTH_FACTOR);
	}
}