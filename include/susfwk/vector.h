// vector.h
//
#ifndef _SUS_VECTOR_
#define _SUS_VECTOR_

#ifdef __cplusplus
extern "C" {
#endif // !__cplusplus

#pragma warning(push)
#pragma warning(disable: 4200)
#pragma warning(disable: 4201)


//////////////////////////////////////////////////////////////////
//						Dynamic buffer							//
//////////////////////////////////////////////////////////////////

// -----------------------------------------------------------

#define SUS_BUFFER_GROWTH_FACTOR 2
#define SUS_SIZE_OF_LARGE 256
#define SUS_BASIC_BUFFER_SIZE 64

// Dynamic Byte buffer
typedef struct sus_buffer{
	SIZE_T	offset;		// Buffer header size
	SIZE_T	capacity;	// Maximum buffer capacity
	SIZE_T	size;		// The size of the memory used
	BYTE	data[];		// Buffer Data
} SUS_BUFFER_STRUCT, *SUS_BUFFER, **SUS_LPBUFFER;

// -------------------------------------

// Create a new buffer
SUS_BUFFER SUSAPI susNewBufferEx(
	_In_ SIZE_T capacity,
	_In_ SIZE_T offset
);
#define susNewBuffer(capacity) susNewBufferEx(capacity, 0)
// Create copy a buffer
SUS_BUFFER SUSAPI susBufferCopy(
	_In_ SUS_BUFFER src,
	_In_ SIZE_T offset
);
// Delete Buffer
SUS_INLINE VOID SUSAPI susBufferDestroy(_Inout_ SUS_BUFFER buff) {
	SUS_ASSERT(buff);
	sus_free((LPBYTE)buff - buff->offset);
}

// -------------------------------------

// Set the buffer data value
SUS_INLINE VOID SUSAPI susBufferSet(_Inout_ SUS_BUFFER buff, _In_ BYTE value) {
	sus_memset(buff->data, buff->capacity, value);
}
// Set the buffer data value to 0
SUS_INLINE VOID SUSAPI susBufferZero(_Inout_ SUS_BUFFER buff) {
	sus_zeromem(buff->data, buff->capacity);
}

// -------------------------------------

// Guaranteed buffer size
VOID SUSAPI susBufferResize(
	_Inout_ SUS_LPBUFFER pBuff,
	_In_ SIZE_T size
);

// -------------------------------------

// Insert data into the buffer with a shift
SUS_LPMEMORY SUSAPI susBufferInsert(
	_Inout_ SUS_LPBUFFER pBuff,
	_In_ SIZE_T pos,
	_In_reads_bytes_opt_(size) CONST LPBYTE data,
	_In_ SIZE_T size
);
// Swap bytes
VOID SUSAPI susBufferSwap(
	_Inout_ SUS_LPBUFFER pBuff,
	_In_ SIZE_T fromPos,
	_In_ SIZE_T toPos,
	_In_ SIZE_T size
);
// Delete data from the buffer
VOID SUSAPI susBufferErase(
	_Inout_ SUS_LPBUFFER pBuff,
	_In_ SIZE_T pos,
	_In_ SIZE_T size
);
// Add data to the end of the buffer
SUS_LPMEMORY SUSAPI susBufferAppend(
	_Inout_ SUS_LPBUFFER pBuff,
	_In_reads_bytes_opt_(size) CONST LPBYTE data,
	_In_ SIZE_T size
);
// Delete data from the end of the buffer
VOID SUSAPI susBufferTruncate(
	_Inout_ SUS_LPBUFFER pBuff,
	_In_ SIZE_T size
);
// Delete all elements of the array
VOID SUSAPI susBufferClear(
	_Inout_ SUS_BUFFER buff
);

// -------------------------------------

#ifndef SUS_DEBUGONLYERRORS
#ifdef _DEBUG
SUS_INLINE VOID SUSAPI susBufferPrint(_In_ SUS_BUFFER buff) {
	SUS_PRINTDL("Buffer output: %s", buff->data);
}
#else
#define susBufferPrint(buff)
#endif // !_DEBUG
#endif // !SUS_DEBUGONLYERRORS

// -------------------------------------

//////////////////////////////////////////////////////////////////
//						Dynamic array							//
//////////////////////////////////////////////////////////////////

// -------------------------------------

// Dynamic array
typedef struct sus_vector {
	SIZE_T	isize;		// The size of the type in the array
	SUS_BUFFER_STRUCT;	// Vector data
} SUS_VECTOR_STRUCT, *SUS_VECTOR, **SUS_LPVECTOR;

// -------------------------------------

// Get the array size
#define susVectorData(array, type) ((type*)((array)->data))
// Get the array size
#define susVectorTypeSize(array) ((array)->isize)
// Get the array count
#define susVectorCount(array) (DWORD)((array)->size / susVectorTypeSize(array))
// Get the array buffer
#define susVectorBuffer(array) ((SUS_BUFFER)((LPBYTE)(array) + array->offset))
// Get the array buffer
#define susVectorSyncBuffer(buff, offset, pVector) *(pVector) = (SUS_VECTOR)((LPBYTE)buff - offset)
// Go through all the elements of the array
#define susVecForeach(start, i, count, vec) for (UINT i = (start), count = susVectorCount(vec); i < count; i++)

// -------------------------------------

// Create a dynamic array
SUS_VECTOR SUSAPI susNewVectorEx(_In_ SIZE_T isize, _In_ SIZE_T offset);
// Create a dynamic array
#define susNewVectorSized(typeSize) susNewVectorEx(typeSize, 0)
// Create a dynamic array
#define susNewVector(type) susNewVectorSized(sizeof(type))
// Destroy the dynamic array
SUS_INLINE VOID SUSAPI susVectorDestroy(_Inout_ SUS_VECTOR array) {
	SUS_ASSERT(array);
	SUS_BUFFER buff = susVectorBuffer(array);
	susBufferDestroy(buff);
}

// -------------------------------------

// Accessing an array element
SUS_INLINE SUS_OBJECT SUSAPI susVectorGet(_Inout_ SUS_VECTOR array, _In_ UINT index) {
	SUS_ASSERT(array && array->size);
	return index >= susVectorCount(array) ? (SUS_OBJECT)NULL : (SUS_OBJECT)(susVectorData(array, BYTE) + index * array->isize);
}
// Accessing an array element
SUS_INLINE SUS_OBJECT SUSAPI susVectorAt(_Inout_ SUS_VECTOR array, _In_ INT index) {
	SUS_ASSERT(array && array->size);
	DWORD count = susVectorCount(array);
	index = index < 0 ? count - (-index % count) : index % count;
	return (SUS_OBJECT)(susVectorData(array, BYTE) + index * array->isize);
}
// Get the first element of the array
SUS_INLINE SUS_OBJECT SUSAPI susVectorFront(_Inout_ SUS_VECTOR array) {
	SUS_ASSERT(array);
	return susVectorData(array, VOID);
}
// Get the last element of the array
SUS_INLINE SUS_OBJECT SUSAPI susVectorBack(_Inout_ SUS_VECTOR array) {
	SUS_ASSERT(array);
	return susVectorAt(array, -1);
}

// -------------------------------------

// Swap bytes
VOID SUSAPI susVectorSwap(
	_Inout_ SUS_LPVECTOR pVector,
	_In_ DWORD from,
	_In_ DWORD to
);
// Add an element to the end of the array
SUS_OBJECT SUSAPI susVectorPushBack(
	_Inout_ SUS_LPVECTOR pVector,
	_In_opt_ SUS_OBJECT object
);
// Add an element to the end of the array
SUS_OBJECT SUSAPI susVectorAppend(
	_Inout_ SUS_LPVECTOR pVector,
	_In_opt_ SUS_OBJECT data,
	_In_ SIZE_T size
);
// Insert an element into an array
SUS_OBJECT SUSAPI susVectorInsert(
	_Inout_ SUS_LPVECTOR pVector,
	_In_ UINT index,
	_In_ SUS_OBJECT object
);


// -------------------------------------

// Delete the last element of the array
VOID SUSAPI susVectorPopBack(
	_Inout_ SUS_LPVECTOR pVector
);
// Remove an element from a dynamic array
VOID SUSAPI susVectorErase(
	_Inout_ SUS_LPVECTOR pVector,
	_In_ DWORD i
);
// Swap places and Delete
SUS_INLINE SUS_OBJECT SUSAPI susVectorSwapErase(_Inout_ SUS_LPVECTOR array, _In_ DWORD i) {
	susVectorSwap(array, i, susVectorCount(*array) - 1);
	susVectorPopBack(array);
	return susVectorGet(*array, i);
}
// Delete all elements of the array
SUS_INLINE VOID SUSAPI susVectorClear(_Inout_ SUS_VECTOR array)
{
	SUS_ASSERT(array);
	susBufferClear(susVectorBuffer(array));
}

// -------------------------------------

#ifdef _DEBUG
SUS_INLINE VOID SUSAPI susVectorPrint(_In_ SUS_VECTOR vec) {
	SUS_PRINTDL("Vector output {");
	susVecForeach(0, i, count, vec) {
		SUS_PRINTDL("'%s'", susVectorGet(vec, i));
	}
	SUS_PRINTDL("}");
}
#else
#define susVectorPrint(vec)
#endif // !_DEBUG

// -------------------------------------

#pragma warning(pop)
#endif /* !_SUS_VECTOR_ */
