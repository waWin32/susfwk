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
#define SUS_HASHMAP_RATIO 0.75f

// Hash table
typedef struct sus_hashmap{
	DWORD		capacity;
	DWORD		count;
	SIZE_T		valueSize;
	SUS_LIST	buckets[];
} SUS_HASHMAP_STRUCT, *SUS_HASHMAP, **SUS_LPHASHMAP;
// The unit of a hash table is a key-value pair
typedef struct sus_mapentry{
	SUS_DATAVIEW	key;
	BYTE			value[];
} SUS_MAPENTRY_STRUCT, *SUS_MAPENTRY, **SUS_LPMAPENTRY;

// Create a hash table
SUS_HASHMAP SUSAPI susNewMapEx(_In_ SIZE_T valueSize, _In_opt_ DWORD initCount);
// Create a hash table
#define susNewMap(valueType) susNewMapEx(sizeof(valueType), 0)
// Get a hash by string
SUS_INLINE UINT SUSAPI susGetHash(SUS_DATAVIEW key) {
	UINT hash = 0;
	for (DWORD i = 0; i < key.size; i++) {
		hash = (hash << 5) - hash + key.data[i];
	}
	return hash;
}
// Get an index in a hash table by key
SUS_INLINE UINT SUSAPI susMapGetIndex(SUS_HASHMAP map, SUS_DATAVIEW key) {
	return susGetHash(key) % map->capacity;
}


// Add a new key-value pair to the hash table
SUS_NODE SUSAPI susMapAdd(
	_Inout_ SUS_LPHASHMAP lpMap,
	_In_ SUS_DATAVIEW key,
	_In_ LPBYTE value
);

#define susMapAddIntKey(lpMap, key, value) do { ULONGLONG __key = (ULONGLONG)key; susMapAdd(lpMap, (SUS_DATAVIEW) { .data = &__key, .size = sizeof(key) }, value); } while (0)
#define susMapAddStringKey(lpMap, key, value) do { susMapAdd(lpMap, (SUS_DATAVIEW) { .data = key, .size = lstrlenA(key) + 1 }, value); } while (0)


// Get an item by key
SUS_NODE SUSAPI susMapGet(
	_In_ SUS_HASHMAP map,
	_In_ SUS_DATAVIEW key
);
// Change the size of the hash table
SUS_HASHMAP SUSAPI susMapCopy(
	_In_ SUS_HASHMAP source,
	_In_ DWORD initCount
);

// Optimizing a hash table by resizing it for improved performance
VOID SUSAPI susMapReserve(
	_Inout_ SUS_LPHASHMAP lpMap
);
// Optimizing a hash table by resizing it for improved performance
VOID SUSAPI susMapCompress(
	_Inout_ SUS_LPHASHMAP lpMap
);

// Delete a key-value pair from a hash table
VOID SUSAPI susMapRemove(
	_In_ SUS_LPHASHMAP lpMap,
	_In_ SUS_DATAVIEW key
);
#define susMapRemoveIntKey(lpMap, key) do { ULONGLONG __key = key; susMapRemove(lpMap, (SUS_DATAVIEW) { .data = &__key, .size = sizeof(key) }); while (0)
#define susMapRemoveStringKey(lpMap, key)  susMapRemove(lpMap, (SUS_DATAVIEW) { .data = key, .size = lstrlenA(key) + 1 })

// Destroy the hash table
SUS_INLINE VOID SUSAPI susMapDestroy(SUS_HASHMAP map) {
	SUS_PRINTDL("Destroying the hash table");
	SUS_ASSERT(map);
	for (DWORD i = 0; i < map->capacity; i++) susListDestroy(map->buckets[i]);
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
	SUS_PRINTDL("Hashmap output {");
	for (DWORD i = 0; i < map->capacity; i++) {
		SUS_LIST bucket = map->buckets[i];
		SUS_PRINTDL("bucket [%d]:", i);
		susListForeach(node, bucket) {
			SUS_MAPENTRY entry = (SUS_MAPENTRY)node->data;
			SUS_PRINTDL("\tkey: '%s' -> '%s' [%d]", entry->key.data, entry->value, susGetHash(entry->key));
		}
	}
	SUS_PRINTDL("Count %d:", map->count);
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