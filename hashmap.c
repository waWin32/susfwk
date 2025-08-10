// hashmap.c
//
#include "coreframe.h"
#include "include/susfwk/core.h"
#include "include/susfwk/memory.h"
#include "include/susfwk/crypto.h"
#include "include/susfwk/linkedList.h"
#include "include/susfwk/hashmap.h"

// Create a hash table
SUS_HASHMAP SUSAPI susNewMapEx(_In_ SIZE_T valueSize, _In_opt_ DWORD initCount)
{
	SUS_PRINTDL("Creating a new hash table");
	SUS_HASHMAP map = sus_malloc(sizeof(SUS_HASHMAP_STRUCT) + (initCount ? initCount : SUS_HASHMAP_INIT_COUNT) * sizeof(SUS_LIST));
	if (!map) return NULL;
	map->capacity = initCount ? initCount : SUS_HASHMAP_INIT_COUNT;
	map->count = 0;
	map->valueSize = valueSize;
	for (DWORD i = 0; i < map->capacity; i++) map->buckets[i] = susNewList();
	return map;
}
// Add a new key-value pair to the hash table
SUS_NODE SUSAPI susMapAdd(_Inout_ SUS_LPHASHMAP lpMap, _In_ SUS_DATAVIEW key, _In_ LPBYTE value)
{
	SUS_ASSERT(!susMapGet(*lpMap, key));
	susMapReserve(lpMap);
	SUS_HASHMAP map = *lpMap;
	SUS_LPLIST bucket = &map->buckets[susMapGetIndex(map, key)];
	SUS_NODE node = susListPushBack(bucket, sizeof(SUS_MAPENTRY_STRUCT) + map->valueSize + key.size);
	if (!node) return NULL;
	SUS_MAPENTRY entry = (SUS_MAPENTRY)node->data;
	if (value) sus_memcpy(entry->value, value, map->valueSize);
	SUS_DATAVIEW keyEntry = { .data = entry->value + map->valueSize, .size = key.size };
	sus_memcpy((LPBYTE)&entry->key, (LPBYTE)&keyEntry, sizeof(keyEntry));
	sus_memcpy(entry->key.data, key.data, key.size);
	map->count++;
	return node;
}
// Get an item by key
SUS_NODE SUSAPI susMapGet(_In_ SUS_HASHMAP map, _In_ SUS_DATAVIEW key)
{
	SUS_LIST bucket = map->buckets[susMapGetIndex(map, key)];
	susListForeach(node, bucket) {
		SUS_MAPENTRY entry = (SUS_MAPENTRY)node->data;
		if (key.size != entry->key.size) continue;
		if (sus_memcmp(entry->key.data, key.data, key.size)) {
			return node;
		}
	}
	return NULL;
}
// Change the size of the hash table
SUS_HASHMAP SUSAPI susMapCopy(_In_ SUS_HASHMAP source, _In_ DWORD initCount)
{
	SUS_HASHMAP newHt = susNewMapEx(source->valueSize, initCount);
	if (!newHt) return NULL;
	for (DWORD i = 0; i < source->capacity; i++) {
		SUS_LIST bucket = source->buckets[i];
		susListForeach(node, bucket) {
			SUS_MAPENTRY kv = (SUS_MAPENTRY)node->data;
			susMapAdd(&newHt, kv->key, kv->value);
		}
	}
	return newHt;
}
// Optimizing a hash table by resizing it for improved performance
VOID SUSAPI susMapReserve(_Inout_ SUS_LPHASHMAP lpMap)
{
	SUS_HASHMAP map = *lpMap;
	if (map->count > map->capacity * SUS_HASHMAP_RATIO) {
		susMapResize(lpMap, map->capacity * SUS_HASHMAP_GROWTH_FACTOR);
	}
}
// Optimizing a hash table by resizing it for improved performance
VOID SUSAPI susMapCompress(_Inout_ SUS_LPHASHMAP lpMap)
{
	SUS_HASHMAP map = *lpMap;
	if (map->count < map->capacity * (1 - SUS_HASHMAP_RATIO) && map->capacity > SUS_HASHMAP_INIT_COUNT) {
		susMapResize(lpMap, map->capacity / SUS_HASHMAP_GROWTH_FACTOR);
	}
}
// Delete a key-value pair from a hash table
VOID SUSAPI susMapRemove(_In_ SUS_LPHASHMAP lpMap, _In_ SUS_DATAVIEW key)
{
	SUS_ASSERT(lpMap && *lpMap && key.data);
	SUS_LPLIST bucket = &(*lpMap)->buckets[susMapGetIndex(*lpMap, key)];
	SUS_NODE node = susMapGet(*lpMap, key);
	if (!node) return;
	susListErase(bucket, node);
	(*lpMap)->count--;
	susMapCompress(lpMap);
}