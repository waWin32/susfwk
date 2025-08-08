// hashmap.h
//
#ifndef _SUS_HASH_TABLE_
#define _SUS_HASH_TABLE_

#ifdef __cplusplus
extern "C" {
#endif // !__cplusplus

#include "linkedlist.h"

#pragma warning(push)
#pragma warning(disable: 4200)

#define SUS_HASHMAP_INIT_COUNT 7
#define SUS_HASHMAP_GROWTH_FACTOR 2
#define SUS_HASHMAP_RATIO 0.75

// Hash table
typedef struct sus_hashmap{
	DWORD	 capacity;
	DWORD	 count;
	SUS_LIST buckets[];
} SUS_HASHMAP_STRUCT, *SUS_HASHMAP, **SUS_LPHASHMAP;
// The unit of a hash table is a key-value pair
typedef struct sus_mapentry{
	LPSTR	key;
	SIZE_T	size;
	BYTE	value[];
} SUS_MAPENTRY_STRUCT, *SUS_MAPENTRY, **SUS_LPMAPENTRY;

// Create a hash table
SUS_HASHMAP SUSAPI susNewMapEx(_In_ DWORD initCount);
// Create a hash table
#define susNewMap() susNewMapEx(0)
// Get a hash by string
SUS_INLINE UINT SUSAPI susGetHash(LPCSTR key) {
	UINT hash = 0;
	while (*key) hash = (hash << 5) - hash + *key++;
	return hash;
}
// Get an index in a hash table by key
SUS_INLINE UINT SUSAPI susMapGetIndex(SUS_HASHMAP map, LPCSTR key) {
	return susGetHash(key) % map->capacity;
}


// Add a new key-value pair to the hash table
SUS_NODE SUSAPI susMapAdd(
	_Inout_ SUS_LPHASHMAP lpMap,
	_In_ LPCSTR key,
	_In_ SUS_DATAVIEW value
);
// Get an item by key
SUS_NODE SUSAPI susMapGet(
	_In_ SUS_HASHMAP map,
	_In_ LPCSTR key
);
// Change the size of the hash table
SUS_HASHMAP SUSAPI susMapCopy(
	_In_ SUS_HASHMAP source,
	_In_ DWORD newCount
);
// Optimizing a hash table by resizing it for improved performance
VOID SUSAPI susMapOptimization(
	_Inout_ SUS_LPHASHMAP lpMap
);

// Delete a key-value pair from a hash table
SUS_INLINE VOID SUSAPI susMapRemove(_In_ SUS_LPHASHMAP lpMap, _In_ LPCSTR key) {
	SUS_ASSERT(lpMap && *lpMap && key);
	SUS_LIST bucket = (*lpMap)->buckets[susMapGetIndex(*lpMap, key)];
	SUS_NODE node = susMapGet(*lpMap, key);
	sus_free(((SUS_MAPENTRY)node->data)->key);
	susListErase(&bucket, node);
	susMapOptimization(lpMap);
	(*lpMap)->count--;
}
// Destroy the hash table
SUS_INLINE VOID SUSAPI susMapDestroy(SUS_HASHMAP map) {
	SUS_PRINTDL("Destroying the hash table");
	SUS_ASSERT(map);
	for (DWORD i = 0; i < map->capacity; i++) {
		susListForeach(node, map->buckets[i]) {
			SUS_MAPENTRY entry = (SUS_MAPENTRY)node->data;
			sus_free(entry->key);
		}
		susListDestroy(map->buckets[i]);
	}
	sus_free(map);
}

// Change the size of the hash table
SUS_INLINE VOID SUSAPI susMapResize(_In_ SUS_LPHASHMAP lpMap, _In_ DWORD newCount) {
	SUS_HASHMAP newHt = susMapCopy(*lpMap, newCount);
	susMapDestroy(*lpMap);
	*lpMap = newHt;
}

#ifdef _DEBUG
SUS_INLINE VOID SUSAPI susMapPrint(_In_ SUS_HASHMAP map) {
	SUS_PRINTDL("HashMap output {");
	for (DWORD i = 0; i < map->capacity; i++) {
		SUS_LIST bucket = map->buckets[i];
		susListForeach(node, bucket) {
			SUS_MAPENTRY entry = (SUS_MAPENTRY)node->data;
			SUS_PRINTDL("\tkey: '%s' -> [%d](%d bytes): '%s'", entry->key, i, entry->size, entry->value);
		}
	}
	SUS_PRINTDL("}");
}
#else
#define susMapPrint(map)
#endif // !_DEBUG

#pragma warning(pop)

#ifdef __cplusplus
}
#endif // !__cplusplus

#endif /* !_SUS_HASH_TABLE_ */