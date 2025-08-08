// vector.c
//
#include "coreframe.h"
#include "include/susfwk/core.h"
#include "include/susfwk/memory.h"
#include "include/susfwk/vector.h"

//////////////////////////////////////////////////////////////////
//						Dynamic buffer							//
//////////////////////////////////////////////////////////////////

// -------------------------------------

// Create a new buffer
SUS_BUFFER SUSAPI susNewBuffer(_In_ SIZE_T capacity, _In_ SIZE_T offset)
{
	capacity = max(capacity, SUS_BASIC_BUFFER_SIZE);
	SUS_BUFFER buffer = (SUS_BUFFER)sus_malloc(sizeof(SUS_BUFFER_HEADER) + capacity + offset);
	if (!buffer) return NULL;
	buffer = (SUS_BUFFER)((LPBYTE)buffer + offset);
	buffer->offset = offset;
	buffer->capacity = capacity;
	buffer->size = 0;
	return buffer;
}
// Create copy a buffer
SUS_BUFFER SUSAPI susBufferCopy(_In_ SUS_BUFFER src, _In_ SIZE_T offset)
{
	SUS_ASSERT(src);
	SUS_BUFFER buff = susNewBuffer(src->size, offset);
	if (!buff) return NULL;
	sus_memcpy(buff->data, src->data, src->size);
	buff->size = src->size;
	return buff;
}

// -------------------------------------

// Guaranteed buffer size
VOID SUSAPI susBufferReserve(
	_Inout_ SUS_LPBUFFER pBuff,
	_In_ SIZE_T required)
{
	SUS_ASSERT(pBuff && *pBuff);
	SUS_BUFFER buff = *pBuff;
	if (buff->capacity >= buff->size + required) return;
	buff->capacity = (!buff->capacity
		? (max(required * SUS_BUFFER_GROWTH_FACTOR, SUS_BASIC_BUFFER_SIZE))
		: (max(buff->capacity * SUS_BUFFER_GROWTH_FACTOR, buff->size + required * SUS_BUFFER_GROWTH_FACTOR))
		);
	SIZE_T offset = buff->offset;
	*pBuff = (SUS_BUFFER)((LPBYTE)sus_realloc((LPBYTE)buff - offset, sizeof(SUS_BUFFER_STRUCT) + buff->capacity + offset) + offset);
}
// Shrink the buffer to the minimum size
VOID SUSAPI susBufferCompress(_Inout_ SUS_LPBUFFER pBuff)
{
	SUS_ASSERT(pBuff && *pBuff);
	SUS_BUFFER buff = *pBuff;
	if (buff->capacity > SUS_SIZE_OF_LARGE && buff->size * SUS_BUFFER_GROWTH_FACTOR * SUS_BUFFER_GROWTH_FACTOR < buff->capacity) {
		buff->capacity = buff->size * SUS_BUFFER_GROWTH_FACTOR;
		SIZE_T offset = buff->offset;
		*pBuff = (SUS_BUFFER)((LPBYTE)sus_realloc((LPBYTE)buff - offset, sizeof(SUS_BUFFER_STRUCT) + buff->capacity + offset) + offset);
	}
}

// -------------------------------------

// Guaranteed buffer size
VOID SUSAPI susBufferResize(
	_Inout_ SUS_LPBUFFER pBuff,
	_In_ SIZE_T size)
{
	SUS_ASSERT(pBuff && *pBuff);
	SUS_BUFFER buff = *pBuff;
	if (size < buff->size) {
		buff->size = size;
		susBufferCompress(pBuff);
		return;
	}
	susBufferReserve(pBuff, size - buff->size);
	buff->size = size;
}
// Insert data into the buffer with a shift
SUS_LPMEMORY SUSAPI susBufferInsert(
	_Inout_ SUS_LPBUFFER pBuff,
	_In_ SIZE_T pos,
	_In_reads_bytes_opt_(size) CONST LPBYTE data,
	_In_ SIZE_T size)
{
	SUS_PRINTDL("Inserting data into a buffer with a shift");
	SUS_ASSERT(pBuff && *pBuff);
	SUS_BUFFER buff = *pBuff;
	pos = min(buff->size, pos);
	susBufferReserve(pBuff, max(0, pos + size - buff->size));
	buff = *pBuff;
	LPBYTE mem = buff->data + pos;
	sus_memmove(mem + size, mem, buff->size - pos);
	if (data) sus_memcpy(mem, data, size);
	buff->size += size;
	return mem;
}
// Delete data from the buffer
VOID SUSAPI susBufferErase(
	_Inout_ SUS_LPBUFFER pBuff,
	_In_ SIZE_T pos,
	_In_ SIZE_T size)
{
	SUS_PRINTDL("Deleting data from the buffer");
	SUS_ASSERT(pBuff && *pBuff && pos < (*pBuff)->size);
	SUS_BUFFER buff = *pBuff;
	if (pos + size > buff->size) size = buff->size - pos;
	LPBYTE mem = buff->data + pos;
	sus_memmove(mem, mem + size, buff->size - pos - size);
	buff->size -= size;
	susBufferCompress(pBuff);
}
// Add data to the end of the buffer
SUS_LPMEMORY SUSAPI susBufferAppend(
	_Inout_ SUS_LPBUFFER pBuff,
	_In_reads_bytes_opt_(size) CONST LPBYTE data,
	_In_ SIZE_T size)
{
	SUS_PRINTDL("Adding an element to the end of an buffer");
	SUS_ASSERT(pBuff && *pBuff);
	SUS_BUFFER buff = *pBuff;
	susBufferReserve(pBuff, size);
	buff = *pBuff;
	LPBYTE mem = buff->data + buff->size;
	if (data) sus_memcpy(mem, data, size);
	buff->size += size;
	return mem;
}
// Delete data from the end of the buffer
VOID SUSAPI susBufferTruncate(
	_Inout_ SUS_LPBUFFER pBuff,
	_In_ SIZE_T size)
{
	SUS_PRINTDL("Deleting data from the end of the buffer");
	SUS_ASSERT(pBuff && *pBuff && (*pBuff)->size >= size);
	SUS_BUFFER buff = *pBuff;
	buff->size -= size;
	susBufferCompress(pBuff);
}
// Delete all elements of the array
VOID SUSAPI susBufferClear(_Inout_ SUS_BUFFER buff)
{
	SUS_PRINTDL("Removing all elements from an buffer");
	SUS_ASSERT(buff);
	buff->size = 0;
}

// -------------------------------------

//////////////////////////////////////////////////////////////////
//						Dynamic array							//
//////////////////////////////////////////////////////////////////

// -------------------------------------

// Create a dynamic array
SUS_VECTOR SUSAPI susNewVectorEx(_In_ SIZE_T isize, _In_ SIZE_T offset) {
	SUS_PRINTDL("Creating a new array");
	offset = SUS_OFFSET_OF(SUS_VECTOR_STRUCT, offset) + offset;
	SUS_BUFFER buff = susNewBuffer(isize * 4, offset);
	if (!buff) return NULL;
	SUS_VECTOR array = (SUS_VECTOR)((LPBYTE)buff - offset);
	array->isize = isize;
	return array;
}

// -------------------------------------

// Add an element to the end of the array
SUS_OBJECT SUSAPI susVectorPushBack(
	_Inout_ SUS_LPVECTOR pVector,
	_In_opt_ SUS_OBJECT object)
{
	SUS_PRINTDL("Adding an element to the end of an array");
	SUS_ASSERT(pVector && *pVector);
	SUS_VECTOR array = *pVector;
	SUS_BUFFER buff = susVectorBuffer(array);
	SIZE_T offset = buff->offset;
	SUS_OBJECT obj = (SUS_OBJECT)susBufferAppend(&buff, object, array->isize);
	susVectorSyncBuffer(buff, offset, pVector);
	return obj;
}
// Insert an element into an array
SUS_OBJECT SUSAPI susVectorInsert(
	_Inout_ SUS_LPVECTOR pVector,
	_In_ UINT index,
	_In_ SUS_OBJECT object)
{
	SUS_PRINTDL("Inserting an element in an array");
	SUS_ASSERT(pVector && *pVector);
	SUS_VECTOR array = *pVector;
	SUS_BUFFER buff = susVectorBuffer(array);
	SIZE_T offset = buff->offset;
	SUS_OBJECT obj = (SUS_OBJECT)susBufferInsert(&buff, index * array->isize, object, array->isize);
	susVectorSyncBuffer(buff, offset, pVector);
	return obj;
}

// -------------------------------------

// Delete the last element of the array
SUS_INLINE VOID SUSAPI susVectorPopBack(
	_Inout_ SUS_LPVECTOR pVector)
{
	SUS_PRINTDL("Deleting the last element from a dynamic array");
	SUS_ASSERT(pVector && *pVector);
	SUS_VECTOR array = *pVector;
	SUS_BUFFER buff = susVectorBuffer(array);
	SIZE_T offset = buff->offset;
	susBufferTruncate(&buff, array->isize);
	susVectorSyncBuffer(buff, offset, pVector);
}
// Remove an element from a dynamic array
SUS_INLINE VOID SUSAPI susVectorErase(
	_Inout_ SUS_LPVECTOR pVector,
	_In_ DWORD i)
{
	SUS_PRINTDL("Deleting an element from a dynamic array");
	SUS_ASSERT(pVector && *pVector);
	SUS_VECTOR array = *pVector;
	SUS_BUFFER buff = susVectorBuffer(array);
	SIZE_T offset = buff->offset;
	susBufferErase(&buff, i * array->isize, array->isize);
	susVectorSyncBuffer(buff, offset, pVector);
}