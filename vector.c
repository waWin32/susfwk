// vector.c
//
#include "coreframe.h"
#include "include/susfwk/core.h"
#include "include/susfwk/memory.h"
#include "include/susfwk/buffer.h"
#include "include/susfwk/vector.h"


// -------------------------------------

// Create a new vector
SUS_VECTOR SUSAPI susNewVectorEx(_In_ sus_size_t itemSize) {
	SUS_PRINTDL("A new array of %d bytes is created", itemSize);
	SUS_ASSERT(itemSize);
	SUS_VECTOR vector = sus_malloc(sizeof(SUS_VECTOR_STRUCT) + SUS_VECTOR_CAPACITY * itemSize);
	if (!vector) return NULL;
	vector->itemSize = itemSize;
	vector->length = 0;
	vector->capacity = SUS_VECTOR_CAPACITY;
	return vector;
}
// Delete a vector
VOID SUSAPI susVectorDestroy(_In_ SUS_VECTOR vector) {
	SUS_PRINTDL("Deleting an array");
	SUS_ASSERT(vector);
	sus_free(vector);
}
// Apply changes to the vector
BOOL SUSAPI susVectorFlush(_Inout_ SUS_LPVECTOR lpVector) {
	SUS_ASSERT(lpVector && *lpVector);
	SUS_VECTOR newVector = (SUS_VECTOR)sus_realloc(*lpVector, sizeof(SUS_VECTOR_STRUCT) + (*lpVector)->capacity * (*lpVector)->itemSize);
	if (!newVector) return FALSE;
	*lpVector = newVector;
	return TRUE;
}

// ---------------------------------------------------------------------------------------

// Reserve elements in an array
static BOOL SUSAPI susVectorReserve(_Inout_ SUS_LPVECTOR lpVector, _In_ sus_uint_t reserve) {
	SUS_ASSERT(lpVector && *lpVector);
	SUS_VECTOR vector = *lpVector;
	if (vector->capacity < vector->length + reserve) {
		vector->capacity = (sus_size32_t)((vector->length + reserve) * SUS_BUFFER_GROW_FACTOR);
		return susVectorFlush(lpVector);
	}
	return TRUE;
}
// Compress the array
static BOOL SUSAPI susVectorCompress(_Inout_ SUS_LPVECTOR lpVector) {
	SUS_ASSERT(lpVector && *lpVector);
	SUS_VECTOR vector = *lpVector;
	if (vector->capacity > SUS_VECTOR_CAPACITY * SUS_BUFFER_GROW_FACTOR && vector->capacity > vector->length * SUS_BUFFER_SHRINK_THRESHOLD) {
		vector->capacity = (sus_size32_t)((sus_float_t)vector->length * SUS_BUFFER_GROW_FACTOR);
		return susVectorFlush(lpVector);
	}
	return TRUE;
}

// -------------------------------------

// Insert elements into an array
SUS_LPMEMORY SUSAPI susVectorInsertArray(_Inout_ SUS_LPVECTOR lpVector, _In_ sus_uint_t i, _In_opt_ SUS_LPMEMORY data, _In_ sus_uint_t count)
{
	SUS_PRINTDL("Inserting elements into the %d index", i);
	SUS_ASSERT(lpVector && *lpVector && i < (*lpVector)->length);
	if (!susVectorReserve(lpVector, count)) return NULL;
	SUS_VECTOR vector = *lpVector;
	sus_size_t byteToMove = (sus_size_t)(vector->length - i) * vector->itemSize;
	if (byteToMove) sus_memmove(vector->data + (sus_size_t)(((sus_size_t)i + count) * vector->itemSize), vector->data + (sus_size_t)(i * vector->itemSize), byteToMove);
	if (data) sus_memcpy(vector->data + (sus_size_t)(i * vector->itemSize), data, count * vector->itemSize);
	else sus_zeromem(vector->data + (sus_size_t)(i * vector->itemSize), count * vector->itemSize);
	vector->length += count;
	return vector->data + i * vector->itemSize;
}
// Set the value of the array elements
SUS_LPMEMORY SUSAPI susVectorSetArray(_Inout_ SUS_LPVECTOR lpVector, _In_ sus_uint_t i, _In_opt_ SUS_LPMEMORY data, _In_ sus_uint_t count)
{
	SUS_PRINTDL("Setting the values of elements with %d indices", i);
	SUS_ASSERT(lpVector && *lpVector && i + count <= (*lpVector)->length);
	SUS_VECTOR vector = *lpVector;
	if (data) sus_memcpy(vector->data + (sus_size_t)(i * vector->itemSize), data, count * vector->itemSize);
	else sus_zeromem(vector->data + (sus_size_t)(i * vector->itemSize), count * vector->itemSize);
	return vector->data + i * vector->itemSize;
}
// Remove elements from an array
BOOL SUSAPI susVectorEraseArray(_Inout_ SUS_LPVECTOR lpVector, _In_ sus_uint_t i, _In_ sus_uint_t count)
{
	SUS_PRINTDL("Deleting array elements from %d", i);
	SUS_ASSERT(lpVector && *lpVector && i + count <= (*lpVector)->length);
	SUS_VECTOR vector = *lpVector;
	sus_size_t byteToMove = (sus_size_t)(vector->length - i - count) * vector->itemSize;
	if (byteToMove) sus_memmove(vector->data + (sus_size_t)(i * vector->itemSize), vector->data + (sus_size_t)(((sus_size_t)i + count) * vector->itemSize), byteToMove);
	vector->length -= count;
	return susVectorCompress(lpVector);
}
// Swap the elements in the array
BOOL SUSAPI susVectorSwap(_In_ SUS_VECTOR vector, _In_ sus_uint_t from, _In_ sus_uint_t to)
{
	SUS_PRINTDL("Replacing element %d with element %d",from, to);
	SUS_ASSERT(vector && from < vector->length && to < vector->length);
	sus_byte_t sbuffer[SUS_BUFFER_TRANSFER_SIZE] = { 0 };
	sus_lpbyte_t buff = sbuffer;
	if (vector->itemSize > SUS_BUFFER_TRANSFER_SIZE) buff = sus_malloc(vector->itemSize);
	if (!buff) return FALSE;
	sus_memcpy(buff, vector->data + (sus_size_t)(from * vector->itemSize), vector->itemSize);
	sus_memcpy(vector->data + (sus_size_t)(from * vector->itemSize), vector->data + (sus_size_t)(to * vector->itemSize), vector->itemSize);
	sus_memcpy(vector->data + (sus_size_t)(to * vector->itemSize), buff, vector->itemSize);
	if (vector->itemSize > SUS_BUFFER_TRANSFER_SIZE) sus_free(buff);
	return TRUE;
}
// Swap places and delete
BOOL SUSAPI susVectorSwapErase(_Inout_ SUS_LPVECTOR lpVector, _In_ sus_uint_t i) {
	SUS_ASSERT(lpVector && *lpVector);
	if (!susVectorSwap(*lpVector, i, (*lpVector)->length - 1)) return FALSE;
	return susVectorPop(lpVector) ? TRUE : FALSE;
}

// Default item search function
static BOOL SUSAPI susVectorDefIndexOfFunc(_In_ SUS_VECTOR vector, _In_ sus_uint_t current, _In_ SUS_OBJECT target) {
	return sus_memcmp(susVectorAt(vector, current), target, vector->itemSize);
}
// Find an element in an array starting from the beginning
sus_int_t SUSAPI susVectorIndexOf(_In_ SUS_VECTOR vector, _In_ SUS_LPMEMORY value, _In_opt_ SUS_VECTOR_INDEXOF_FUNC func) {
	SUS_ASSERT(vector && value);
	if (!func) func = susVectorDefIndexOfFunc;
	susVecForeach(i, vector) {
		if (func(vector, i, value)) return i;
	}
	return -1;
}
// Find an array element starting from the end
sus_int_t SUSAPI susVectorLastIndexOf(_In_ SUS_VECTOR vector, _In_ SUS_LPMEMORY value, _In_opt_ SUS_VECTOR_INDEXOF_FUNC func) {
	SUS_ASSERT(vector && value);
	if (!func) func = susVectorDefIndexOfFunc;
	susVecForeachReverse(i, vector) {
		if (func(vector, i, value)) return i;
	}
	return -1;
}

// -------------------------------------
