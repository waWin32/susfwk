// hashtable.h
//
#ifndef _SUS_HASH_TABLE_
#define _SUS_HASH_TABLE_

#ifdef __cplusplus
extern "C" {
#endif // !__cplusplus

#include "vector.h"

#pragma warning(push)
#pragma warning(disable: 4200)

// ================================================================================================

// Hash type
typedef DWORD SUS_HASH_T;
// Callback function for data hashing
typedef SUS_HASH_T(SUSAPI* SUS_GET_HASH)(SUS_DATAVIEW key);

#define	SUS_FNV_OFFSET (SUS_HASH_T)2166136261u
#define	SUS_FNV_PRIME (SUS_HASH_T)16777619u

// Get a hash by key
SUS_INLINE SUS_HASH_T SUSAPI susDefGetHash(SUS_DATAVIEW key) {
	SUS_ASSERT(key.data);
	SUS_HASH_T hash = SUS_FNV_OFFSET;
	DWORD i = 0;
	for (; i + 4 <= (DWORD)key.size; i += 4) {
		hash ^= key.data[i]; hash *= SUS_FNV_PRIME;
		hash ^= key.data[i + 1]; hash *= SUS_FNV_PRIME;
		hash ^= key.data[i + 2]; hash *= SUS_FNV_PRIME;
		hash ^= key.data[i + 3]; hash *= SUS_FNV_PRIME;
	}
	for (; i < key.size; i++) {
		hash ^= key.data[i];
		hash *= SUS_FNV_PRIME;
	}
	return hash;
}
// Get a hash by key
SUS_INLINE SUS_HASH_T SUSAPI susDefGetHashInt(SUS_DATAVIEW key) {
	SUS_ASSERT(key.data);
	return *(SUS_HASH_T*)key.data;
}

#define SUS_HASHTABLE_INIT_COUNT 7
#define SUS_HASHTABLE_GROWTH_FACTOR 2
#define SUS_HASHTABLE_RATIO 0.75f

// ================================================================================================


// -------------------------------------------------------------------

// Hash table
typedef struct sus_hashmap{
	SUS_GET_HASH	getHash;	// Hashing function
	SIZE_T			keySize;	// Key size in bytes
	SIZE_T			valueSize;	// Value size in bytes
	DWORD			capacity;	// Number of buckets
	DWORD			count;		// Total number of table elements
	SUS_VECTOR		buckets[];	// Buckets
} SUS_HASHMAP_STRUCT, *SUS_HASHMAP, **SUS_LPHASHMAP;

// -------------------------------------------------------------------

// Create a hash table
SUS_HASHMAP SUSAPI susNewMapEx(_In_ SIZE_T keySize, _In_ SIZE_T valueSize, _In_opt_ SUS_GET_HASH getHash, _In_opt_ DWORD initCount);
// Create a hash table
#define susNewMapSized(keySize, valueSize) susNewMapEx(keySize, valueSize, NULL, 0)
// Create a hash table
#define susNewMap(keyType, valueType) susNewMapSized(sizeof(keyType), sizeof(valueType))
// Destroy the hash table
SUS_INLINE VOID SUSAPI susMapDestroy(SUS_HASHMAP map) {
	SUS_PRINTDL("Destroying the hash table");
	SUS_ASSERT(map);
	for (DWORD i = 0; i < map->capacity; i++) susVectorDestroy(map->buckets[i]);
	sus_free(map);
}
// Change the size of the hash table
SUS_HASHMAP SUSAPI susMapCopy(
	_In_ SUS_HASHMAP source,
	_In_ DWORD initCount
);

// -------------------------------------------------------------------

// Optimizing a hash table by resizing it for improved performance
VOID SUSAPI susMapReserve(
	_Inout_ SUS_LPHASHMAP lpMap
);
// Optimizing a hash table by resizing it for improved performance
VOID SUSAPI susMapCompress(
	_Inout_ SUS_LPHASHMAP lpMap
);
// Change the size of the hash table
SUS_INLINE VOID SUSAPI susMapResize(_In_ SUS_LPHASHMAP lpMap, _In_ DWORD newCount) {
	SUS_HASHMAP newHt = susMapCopy(*lpMap, newCount);
	if (!newHt) return;
	susMapDestroy(*lpMap);
	*lpMap = newHt;
}

// -------------------------------------------------------------------

// Get an index in a hash table by key
SUS_INLINE DWORD SUSAPI susMapGetIndex(SUS_HASHMAP map, LPBYTE key) {
	return map->getHash((SUS_DATAVIEW) { .data = key, map->keySize }) % map->capacity;
}
// Get an item by key
SUS_OBJECT SUSAPI susMapGet(
	_In_ SUS_HASHMAP map,
	_In_bytecount_(map->valueSize) SUS_OBJECT key
);
// Get the entry from the hash table node
#define susMapEntry(bucket, i) (SUS_OBJECT)susVectorGet(bucket, i)
// Get the key from the hash table node
#define susMapKey(map, entry) (entry)
// Get the value from the hash table node
#define susMapValue(map, entry) ((SUS_OBJECT)((entry) + ((SUS_HASHMAP)(map))->keySize))
// Iterate over all elements of the hash table
#define susMapForeach(map, tag, entry) for (DWORD __i = 0; __i < map->capacity; __i++) susVecForeach(0, __j, tag, (map)->buckets[__i]) for (LPBYTE entry = (LPBYTE)susVectorGet((map)->buckets[__i], __j); entry; entry = NULL)

// -------------------------------------------------------------------

// Add a new key-value pair to the hash table
SUS_OBJECT SUSAPI susMapAdd(
	_Inout_ SUS_LPHASHMAP lpMap,
	_In_bytecount_((*lpMap)->keySize) SUS_OBJECT key,
	_In_opt_bytecount_((*lpMap)->valueSize) SUS_OBJECT value
);

// Delete a key-value pair from a hash table
VOID SUSAPI susMapRemove(
	_Inout_ SUS_LPHASHMAP lpMap,
	_In_bytecount_((*lpMap)->keySize) SUS_OBJECT key
);
// Clearing all hash table elements
VOID SUSAPI susMapClear(
	_In_ SUS_HASHMAP map
);

// -------------------------------------------------------------------

#ifndef SUS_DEBUGONLYERRORS
#ifdef _DEBUG
SUS_INLINE VOID SUSAPI susMapPrint(_In_ SUS_HASHMAP map) {
	SUS_PRINTDL("Hashmap output {");
	for (DWORD i = 0; i < map->capacity; i++) {
		SUS_VECTOR bucket = map->buckets[i];
		SUS_PRINTDL("bucket [%d]:", i);
		susVecForeach(0, j, _count, bucket) {
			LPBYTE entry = (LPBYTE)susMapEntry(bucket, j);
			SUS_PRINTDL("\tkey: '%s' -> '%s'", susMapKey(map, entry), susMapValue(map, entry));
		}
	}
	SUS_PRINTDL("Count %d:", map->count);
	SUS_PRINTDL("}");
}
#else
#define susMapPrint(map)
#endif // !_DEBUG
#endif // !SUS_DEBUGONLYERRORS

// ================================================================================================


// An unordered array
typedef SUS_HASHMAP_STRUCT SUS_HASHSET_STRUCT, *SUS_HASHSET, **SUS_LPHASHSET;

#define susNewSetSized(typeSize)	(SUS_HASHSET)susNewMapSized(typeSize, 0);
#define susNewSet(type)				susNewSetSized(sizeof(type));
#define susSetCopy					(SUS_HASHSET)susMapCopy
#define susSetDestroy				susMapDestroy

#define susSetForeach(set, entry)	susMapForeach(set, entry)
#define susSetContains(set, key)	((BOOL)(susMapGet(set, key) != NULL))
#define susSetClear					susMapClear
#define susSetValue(set, entry)		susMapKey(set, entry)
#define susSetAdd(lpSet, key)		susMapAdd(lpSet, key, NULL)
#define susSetRemove(lpSet, key)	susMapRemove(lpSet, key)

#pragma warning(pop)

#ifdef __cplusplus
}
#endif // !__cplusplus

#endif /* !_SUS_HASH_TABLE_ */