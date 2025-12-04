// hashtable.c
//
#include "coreframe.h"
#include "include/susfwk/core.h"
#include "include/susfwk/memory.h"
#include "include/susfwk/vector.h"
#include "include/susfwk/hashtable.h"

// -------------------------------------------------------------------

// Create a hash table
SUS_HASHMAP SUSAPI susNewMapEx(_In_ SIZE_T keySize, _In_ SIZE_T valueSize, _In_opt_ SUS_GET_HASH_CALLBACK getHash, _In_opt_ SUS_CMP_KEYS_CALLBACK cmpKeys, _In_opt_ DWORD initCount)
{
	SUS_PRINTDL("Creating a new hash table");
	SUS_ASSERT(keySize);
	SUS_HASHMAP map = sus_malloc(sizeof(SUS_HASHMAP_STRUCT) + (initCount ? initCount : SUS_HASHTABLE_INIT_COUNT) * sizeof(SUS_VECTOR));
	if (!map) return NULL;
	map->capacity = initCount ? initCount : SUS_HASHTABLE_INIT_COUNT;
	map->count = 0;
	map->valueSize = (DWORD)valueSize;
	map->keySize = (DWORD)keySize;
	map->getHash = getHash ? getHash : (keySize <= 4 ? susDefGetHashInt : susDefGetHash);
	map->cmpKeys = cmpKeys ? cmpKeys : susDefCmpKeys;
	for (DWORD i = 0; i < map->capacity; i++) map->buckets[i] = susNewVectorSized(keySize + valueSize);
	return map;
}
// Change the size of the hash table
SUS_HASHMAP SUSAPI susMapCopy(_In_ SUS_HASHMAP source, _In_ DWORD initCount)
{
	SUS_PRINTDL("Copying a hash table");
	SUS_ASSERT(source);
	SUS_HASHMAP map = susNewMapEx(source->keySize, source->valueSize, source->getHash, source->cmpKeys, initCount);
	if (!map) return NULL;
	susMapForeach(source, i) {
		susMapAdd(&map, susMapIterKey(i), susMapIterValue(i));
	}
	return map;
}

// -------------------------------------------------------------------

// Optimizing a hash table by resizing it for improved performance
VOID SUSAPI susMapReserve(_Inout_ SUS_LPHASHMAP lpMap)
{
	SUS_ASSERT(lpMap && *lpMap);
	SUS_HASHMAP map = *lpMap;
	if ((map->count + 1) >= map->capacity * SUS_HASHTABLE_RATIO) {
		susMapResize(lpMap, map->capacity * SUS_HASHTABLE_GROWTH_FACTOR);
	}
}
// Optimizing a hash table by resizing it for improved performance
VOID SUSAPI susMapCompress(_Inout_ SUS_LPHASHMAP lpMap)
{
	SUS_ASSERT(lpMap && *lpMap);
	SUS_HASHMAP map = *lpMap;
	if (map->count < map->capacity * (1 - SUS_HASHTABLE_RATIO) && map->capacity > SUS_HASHTABLE_INIT_COUNT) {
		susMapResize(lpMap, map->capacity / SUS_HASHTABLE_GROWTH_FACTOR);
	}
}

// -------------------------------------------------------------------

// Get an item by key
SUS_OBJECT SUSAPI susMapGetEntry(_In_ SUS_HASHMAP map, _In_bytecount_(map->valueSize) SUS_OBJECT key)
{
	SUS_PRINTDL("Getting a node from a hash table");
	SUS_ASSERT(map && key);
	SUS_VECTOR bucket = map->buckets[susMapGetIndex(map, key)];
	susVecForeach(i, bucket) {
		LPBYTE entry = (LPBYTE)susVectorGet(bucket, i);
		if (map->cmpKeys(susMapKey(map, entry), key, map->keySize)) {
			return entry;
		}
	}
	return NULL;
}
// Add a new key-value pair to the hash table
SUS_OBJECT SUSAPI susMapAdd(_Inout_ SUS_LPHASHMAP lpMap, _In_bytecount_((*lpMap)->keySize) SUS_OBJECT key, _In_opt_bytecount_((*lpMap)->valueSize) SUS_OBJECT value)
{
	SUS_PRINTDL("Adding a new key-value pair to a hash table");
	SUS_ASSERT(lpMap && *lpMap && key && !susMapGetEntry(*lpMap, key));
	susMapReserve(lpMap);
	SUS_HASHMAP map = *lpMap;
	SUS_LPVECTOR bucket = &map->buckets[susMapGetIndex(map, key)];
	LPBYTE entry = susVectorPushBack(bucket, NULL);
	if (!entry) return NULL;
	sus_memcpy(susMapKey(map, entry), key, map->keySize);
	if (value) sus_memcpy(susMapValue(map, entry), value, map->valueSize);
	else sus_zeromem(susMapValue(map, entry), map->valueSize);
	map->count++;
	return susMapValue(map, entry);
}
// Add or change a value
SUS_OBJECT SUSAPI susMapSet(_Inout_ SUS_LPHASHMAP lpMap, _In_bytecount_((*lpMap)->keySize) SUS_OBJECT key, _In_opt_bytecount_((*lpMap)->valueSize) SUS_OBJECT value)
{
	SUS_ASSERT(lpMap && *lpMap && key);
	SUS_OBJECT mvalue = susMapGet(*lpMap, key);
	if (!mvalue) mvalue = susMapAdd(lpMap, key, value);
	else if (value) sus_memcpy(mvalue, value, (*lpMap)->valueSize); else sus_zeromem(mvalue, (*lpMap)->valueSize);
	return mvalue;
}
// Delete a key-value pair from a hash table
VOID SUSAPI susMapRemove(_Inout_ SUS_LPHASHMAP lpMap, _In_bytecount_((*lpMap)->keySize) SUS_OBJECT key)
{
	SUS_PRINTDL("Deleting an item from a table");
	SUS_ASSERT(lpMap && *lpMap && key && susMapGet(*lpMap, key));
	SUS_HASHMAP map = *lpMap;
	SUS_LPVECTOR bucket = &map->buckets[susMapGetIndex(map, key)];
	susVecForeach(i, *bucket) {
		LPBYTE entry = (LPBYTE)susVectorGet(*bucket, i);
		if (map->cmpKeys(susMapKey(map, entry), key, map->keySize)) {
			susVectorErase(bucket, i);
			map->count--;
			susMapCompress(lpMap);
			return;
		}
	}
}
// Clearing all hash table elements
VOID SUSAPI susMapClear(_In_ SUS_HASHMAP map)
{
	for (DWORD i = 0; i < map->capacity; i++) {
		susVectorDestroy(map->buckets[i]);
	}
	map->count = 0;
}

// -------------------------------------------------------------------

// Start getting data from the hash table
SUS_MAP_ITER SUSAPI susMapIterBegin(_In_ SUS_HASHMAP map)
{
	SUS_ASSERT(map);
	SUS_MAP_ITER iter = { .map = map, .bucketIndex = 0, .entryIndex = 0 };
	while (iter.bucketIndex < iter.map->capacity && !iter.map->buckets[iter.bucketIndex]->size) iter.bucketIndex++;
	return iter;
}
// Go to the next element in the hash table
BOOL SUSAPI susMapIterNext(_Inout_ SUS_LPMAP_ITER iter)
{
	SUS_ASSERT(iter && iter->map);
	if (iter->count >= iter->map->count) return FALSE;
	iter->count++;
	if (iter->entryIndex + 1 < iter->map->buckets[iter->bucketIndex]->length) {
		iter->entryIndex++;
		return TRUE;
	}
	iter->entryIndex = 0;
	do iter->bucketIndex++; while (iter->bucketIndex < iter->map->capacity && !iter->map->buckets[iter->bucketIndex]->size);
	return iter->bucketIndex < iter->map->capacity;
}

// -------------------------------------------------------------------
