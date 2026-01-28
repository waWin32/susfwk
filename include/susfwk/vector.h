// vector.h
//
#ifndef _SUS_VECTOR_
#define _SUS_VECTOR_

#include "buffer.h"

#pragma warning(push)
#pragma warning(disable: 4200)
#pragma warning(disable: 4201)

// -------------------------------------

// Default base vector size
#define SUS_VECTOR_CAPACITY 4
// Static size for transfer vector
#define SUS_VECTOR_TRANSFER_SIZE 12
// Dynamic array
typedef struct sus_vector {
	sus_uint_t	length;		// Length of the array
	sus_uint_t	capacity;	// Vector capacity in elements
	sus_size_t	itemSize;	// The size of the element in bytes
	sus_byte_t	data[];		// Array data
} SUS_VECTOR_STRUCT, *SUS_VECTOR, **SUS_LPVECTOR;

// -------------------------------------

// Create a new vector
SUS_VECTOR SUSAPI susNewVectorEx(_In_ sus_size_t itemSize);
//
#define susNewVector(type) susNewVectorEx(sizeof(type))
// Delete a vector
VOID SUSAPI susVectorDestroy(_In_ SUS_VECTOR vector);
// Apply changes to the vector
BOOL SUSAPI susVectorFlush(_Inout_ SUS_LPVECTOR vector);

// -------------------------------------

// Insert elements into an array
SUS_LPMEMORY SUSAPI susVectorInsertArray(_Inout_ SUS_LPVECTOR lpVector, _In_ sus_uint_t i, _In_opt_ SUS_LPMEMORY data, _In_ sus_uint_t count);
// Set the value of the array elements
SUS_LPMEMORY SUSAPI susVectorSetArray(_Inout_ SUS_LPVECTOR lpVector, _In_ sus_uint_t i, _In_opt_ SUS_LPMEMORY data, _In_ sus_uint_t count);
// Remove elements from an array
BOOL SUSAPI susVectorEraseArray(_Inout_ SUS_LPVECTOR lpVector, _In_ sus_uint_t i, _In_ sus_uint_t count);
// Swap the elements in the array
BOOL SUSAPI susVectorSwap(_In_ SUS_VECTOR vector, _In_ sus_uint_t from, _In_ sus_uint_t to);
// Swap places and delete
BOOL SUSAPI susVectorSwapErase(_Inout_ SUS_LPVECTOR lpVector, _In_ sus_uint_t i);

// The function for 'indexOf' operation
typedef BOOL(SUSAPI* SUS_VECTOR_INDEXOF_FUNC)(_In_ SUS_VECTOR vector, _In_ sus_uint_t current, _In_ SUS_OBJECT target);
// Find an element in an array starting from the beginning
sus_int_t SUSAPI susVectorIndexOf(_In_ SUS_VECTOR vector, _In_ SUS_LPMEMORY value, _In_opt_ SUS_VECTOR_INDEXOF_FUNC func);
// Find an array element starting from the end
sus_int_t SUSAPI susVectorLastIndexOf(_In_ SUS_VECTOR vector, _In_ SUS_LPMEMORY value, _In_opt_ SUS_VECTOR_INDEXOF_FUNC func);

// -------------------------------------

// Set an element to an array
SUS_INLINE SUS_LPMEMORY SUSAPI susVectorInsert(_Inout_ SUS_LPVECTOR lpVector, _In_ sus_uint_t i, _In_opt_ SUS_LPMEMORY data) { return susVectorInsertArray(lpVector, i, data, 1); }
// Delete an element from an array
SUS_INLINE BOOL SUSAPI susVectorErase(_Inout_ SUS_LPVECTOR lpVector, _In_ sus_uint_t i) { return susVectorEraseArray(lpVector, i, 1); }
// 
SUS_INLINE SUS_LPMEMORY SUSAPI susVectorSet(_Inout_ SUS_LPVECTOR lpVector, _In_ sus_uint_t i, _In_opt_ SUS_LPMEMORY data) { return susVectorSetArray(lpVector, i, data, 1); }

// -------------------------------------

// Insert an element at the end of the array
SUS_INLINE SUS_LPMEMORY SUSAPI susVectorPush(_Inout_ SUS_LPVECTOR lpVector, _In_opt_ SUS_LPMEMORY data) { return susVectorInsertArray(lpVector, (*lpVector)->length, data, 1); }
// Delete an element from the end of the array
SUS_INLINE BOOL SUSAPI susVectorPop(_Inout_ SUS_LPVECTOR lpVector) { SUS_ASSERT(lpVector && *lpVector && (*lpVector)->length); return susVectorEraseArray(lpVector, (*lpVector)->length - 1, 1); }

// Insert an element at the beginning of the array
SUS_INLINE SUS_LPMEMORY SUSAPI susVectorUnshift(_Inout_ SUS_LPVECTOR lpVector, _In_opt_ SUS_LPMEMORY data) { SUS_ASSERT(lpVector && *lpVector); return susVectorInsertArray(lpVector, 0, data, 1); }
// Delete the first element of the array
SUS_INLINE BOOL SUSAPI susVectorShift(_Inout_ SUS_LPVECTOR lpVector) { SUS_ASSERT(lpVector && *lpVector); return susVectorEraseArray(lpVector, 0, 1); }

// -------------------------------------

// Get a pointer to the array data
#define susVectorAt(vector, i) ((SUS_LPMEMORY)((vector)->data + (i) * (vector)->itemSize))
// Get the array data
#define susVectorGet(vector, i, type) (*(type*)susVectorAt(vector, i))
// Get the array data
#define susVectorContains(vector, value, func) ((BOOL)(susVectorIndexOf(vector, value, func) ? TRUE : FALSE))
// Walk through the array
#define susVecForeach(i, vector) for (sus_uint_t i = 0; i < (vector)->length; i++)
// Walk through the array revers
#define susVecForeachReverse(i, vector) for (sus_int_t i = (vector)->length - 1; i >= 0; i--)

// -------------------------------------

#pragma warning(pop)

#endif /* !_SUS_VECTOR_ */
