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
SUS_BUFFER SUSAPI susNewBufferEx(_In_ SIZE_T capacity, _In_ SIZE_T offset)
{
	capacity = max(capacity, SUS_BASIC_BUFFER_SIZE);
	SUS_BUFFER buffer = (SUS_BUFFER)sus_malloc(sizeof(SUS_BUFFER_STRUCT) + capacity + offset);
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
	SUS_BUFFER buff = susNewBufferEx(src->size, offset);
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

// -------------------------------------

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
// Swap bytes
VOID SUSAPI susBufferSwap(
	_Inout_ SUS_BUFFER buff,
	_In_ SIZE_T fromPos,
	_In_ SIZE_T toPos,
	_In_ SIZE_T size)
{
	SUS_PRINTDL("Swapping the byte buffer");
	SUS_ASSERT(buff && size);
	SUS_ASSERT(!(fromPos + size > buff->size) && !(toPos + size > buff->size));
	if (fromPos == toPos) return;
	SUS_ASSERT(!((toPos >= fromPos && toPos < fromPos + size) || (fromPos >= toPos && fromPos < toPos + size)));
	LPBYTE tmp = (LPBYTE)sus_malloc(size);
	if (!tmp) return;
	sus_memcpy(tmp, buff->data + fromPos, size);
	sus_memcpy(buff->data + fromPos, buff->data + toPos, size);
	sus_memcpy(buff->data + toPos, tmp, size);
	sus_free(tmp);
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
	else sus_zeromem(mem, size);
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
SUS_VECTOR SUSAPI susNewVectorEx(_In_ SIZE_T itemSize) {
	SUS_PRINTDL("Creating a new array");
	SIZE_T offset = SUS_OFFSET_OF(SUS_VECTOR_STRUCT, offset);
	SUS_BUFFER buff = susNewBufferEx(itemSize * 4, offset);
	if (!buff) return NULL;
	SUS_VECTOR array = (SUS_VECTOR)((LPBYTE)buff - offset);
	array->itemSize = (DWORD)itemSize;
	array->length = 0;
	return array;
}

// -------------------------------------

// Swap bytes
VOID SUSAPI susVectorSwap(
	_Inout_ SUS_VECTOR vector,
	_In_ DWORD from,
	_In_ DWORD to)
{
	SUS_PRINTDL("Replacing a vector array");
	SUS_ASSERT(vector);
	SUS_BUFFER buff = susVectorBuffer(vector);
	SIZE_T offset = buff->offset;
	susBufferSwap(buff, from * vector->itemSize, to * vector->itemSize, vector->itemSize);
	susVectorSyncBuffer(buff, offset, &vector);
}
// Add an element to the end of the array
SUS_OBJECT SUSAPI susVectorPushBack(
	_Inout_ SUS_LPVECTOR pVector,
	_In_opt_ SUS_OBJECT object)
{
	SUS_PRINTDL("Adding an element to the end of an array");
	SUS_ASSERT(pVector && *pVector);
	SUS_VECTOR vector = *pVector;
	SUS_BUFFER buff = susVectorBuffer(vector);
	SIZE_T offset = buff->offset;
	vector->length++;
	SUS_OBJECT obj = (SUS_OBJECT)susBufferAppend(&buff, object, vector->itemSize);
	susVectorSyncBuffer(buff, offset, pVector);
	return obj;
}
// Add an element to the end of the array
SUS_OBJECT SUSAPI susVectorAppend(
	_Inout_ SUS_LPVECTOR pVector,
	_In_opt_ SUS_OBJECT data,
	_In_ DWORD count)
{
	SUS_PRINTDL("Adding an element to the end of an array");
	SUS_ASSERT(pVector && *pVector);
	SUS_VECTOR vector = *pVector;
	SUS_BUFFER buff = susVectorBuffer(vector);
	SIZE_T offset = buff->offset;
	vector->length += (DWORD)((count * vector->itemSize + vector->itemSize - 1) / vector->itemSize);
	SUS_OBJECT obj = (SUS_OBJECT)susBufferAppend(&buff, data, count * vector->itemSize);
	susVectorSyncBuffer(buff, offset, pVector);
	return obj;
}
// Insert an element into an array
SUS_OBJECT SUSAPI susVectorInsert(
	_Inout_ SUS_LPVECTOR pVector,
	_In_ UINT index,
	_In_opt_ SUS_OBJECT object)
{
	SUS_PRINTDL("Inserting an element in an array");
	SUS_ASSERT(pVector && *pVector);
	SUS_VECTOR vector = *pVector;
	SUS_BUFFER buff = susVectorBuffer(vector);
	SIZE_T offset = buff->offset;
	vector->length++;
	SUS_OBJECT obj = (SUS_OBJECT)susBufferInsert(&buff, index * vector->itemSize, object, vector->itemSize);
	susVectorSyncBuffer(buff, offset, pVector);
	return obj;
}

// -------------------------------------

// The default vector element comparison function
static INT SUSAPI susDefVectorSearcher(_In_ SUS_OBJECT obj, _In_ SUS_OBJECT sought, _In_ SIZE_T size) {
	return sus_memcmp(obj, sought, size) ? 0 : 1;
}
// Find the first element and get the index
INT SUSAPI susVectorIndexOf(_In_ SUS_VECTOR vector, _In_ SUS_OBJECT obj, _In_opt_ SUS_VECTOR_ELEMENTS_COMPARE searcher) {
	SUS_ASSERT(vector && obj);
	searcher = searcher ? searcher : susDefVectorSearcher;
	susVecForeach(i, vector) {
		if (searcher(susVectorGet(vector, i), obj, vector->itemSize) == 0) return i;
	}
	return -1;
}
// Find the last element and get the index
INT SUSAPI susVectorLastIndexOf(_In_ SUS_VECTOR vector, _In_ SUS_OBJECT obj, _In_opt_ SUS_VECTOR_ELEMENTS_COMPARE searcher) {
	SUS_ASSERT(vector && obj);
	searcher = searcher ? searcher : susDefVectorSearcher;
	susVecForeachReverse(i, vector) {
		if (searcher(susVectorGet(vector, i), obj, vector->itemSize) == 0) return i;
	}
	return -1;
}

// The default vector element sorter function
static BOOL SUSAPI susDefVectorSorter(_In_ SUS_VECTOR vector, _In_ UINT a, _In_ UINT b) {
	return *((int*)susVectorGet(vector, a)) > *((int*)susVectorGet(vector, b));
}
// The first part of sorting by the lomuto method
static UINT susVectorSortPart(_In_ SUS_VECTOR vector, _In_ UINT start, UINT end, _In_ SUS_VECTOR_ELEMENTS_SORTER sorter) {
	SUS_ASSERT(sorter);
	susVectorSwap(vector, start + (end - start) / 2, end);
	for (UINT current = start; current < end; current++) {
		if (sorter(vector, end, current)) {
			susVectorSwap(vector, current, start);
			start++;
		}
	}
	susVectorSwap(vector, end, start);
	return start;
}
// Lomuto sorting
VOID SUSAPI susVectorSort(_In_ SUS_VECTOR vector, _In_ UINT start, _In_ UINT end, _In_opt_ SUS_VECTOR_ELEMENTS_SORTER sorter)
{
	if (end <= start) return;
	if (!sorter) sorter = susDefVectorSorter;
	UINT right = susVectorSortPart(vector, start, end, sorter);
	susVectorSort(vector, start, right - 1, sorter);
	susVectorSort(vector, right + 1, end, sorter);
}

// -------------------------------------

// Delete the last element of the array
VOID SUSAPI susVectorPopBack(
	_Inout_ SUS_LPVECTOR pVector)
{
	SUS_PRINTDL("Deleting the last element from a dynamic array");
	SUS_ASSERT(pVector && *pVector);
	SUS_VECTOR vector = *pVector;
	SUS_BUFFER buff = susVectorBuffer(vector);
	SIZE_T offset = buff->offset;
	vector->length--;
	susBufferTruncate(&buff, vector->itemSize);
	susVectorSyncBuffer(buff, offset, pVector);
}
// Remove an element from a dynamic array
VOID SUSAPI susVectorErase(
	_Inout_ SUS_LPVECTOR pVector,
	_In_ DWORD i)
{
	SUS_PRINTDL("Deleting an element from a dynamic array");
	SUS_ASSERT(pVector && *pVector);
	SUS_VECTOR vector = *pVector;
	SUS_BUFFER buff = susVectorBuffer(vector);
	SIZE_T offset = buff->offset;
	vector->length--;
	susBufferErase(&buff, i * vector->itemSize, vector->itemSize);
	susVectorSyncBuffer(buff, offset, pVector);
}