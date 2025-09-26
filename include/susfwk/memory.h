// memory.h
//
#ifndef _SUS_MEMORY_
#define _SUS_MEMORY_

#ifdef __cplusplus
extern "C" {
#endif // !__cplusplus

	// Number of attempts to allocate memory
#define SUS_NUMBER_ATTEMPTS_ALLOCATE_MEMORY 0xff

// Dynamic data type
typedef LPVOID SUS_DYNAMIC, *SUS_PDYNAMIC, *SUS_LPDYNAMIC;
typedef LPVOID SUS_OBJECT, *SUS_POBJECT, *SUS_LPOBJECT;
typedef LPVOID SUS_LPMEMORY;

typedef SUS_FILE SUS_HEAP, *SUS_PHEAP, *SUS_LPHEAP;
typedef SUS_FILE SUS_MEMORY;

//////////////////////////////////////////////////////////////////
//					Basic memory operations						//
//////////////////////////////////////////////////////////////////

// Initialize a memory block
SUS_INLINE VOID SUSAPI sus_memset(
	_Out_writes_bytes_all_(size) LPBYTE data,
	_In_ SIZE_T size,
	_In_ BYTE value)
{
	SUS_ASSERT(data);
	__stosb(data, value, size);
}
// Initialize a memory block with zeros
SUS_INLINE VOID SUSAPI sus_zeromem(
	_Out_writes_bytes_all_(size) LPBYTE data,
	_In_ SIZE_T size)
{
	SUS_ASSERT(data);
	__stosb(data, 0, size);
}
// Comparing memory blocks
SUS_INLINE BOOL SUSAPI sus_memcmp(
	_In_bytecount_(size) CONST LPBYTE lpBuf1,
	_In_bytecount_(size) CONST LPBYTE lpBuf2,
	_In_ SIZE_T size)
{
	SUS_ASSERT(lpBuf1 && lpBuf2);
	return RtlCompareMemory(lpBuf1, lpBuf2, size) == size ? TRUE : FALSE;
}
// Copy the memory
SUS_INLINE VOID SUSAPI sus_memcpy(
	_Out_writes_bytes_all_(size) LPBYTE buff,
	_In_reads_bytes_(size) CONST LPBYTE source,
	_In_ SIZE_T size)
{
	SUS_ASSERT(buff && source && !(buff > source && buff < source + size));
	__movsb(buff, source, size);
}
// Copy the memory
SUS_INLINE VOID SUSAPI sus_memmove(
	_Out_writes_bytes_all_(size) LPBYTE buff,
	_In_reads_bytes_(size) CONST LPBYTE source,
	_In_ SIZE_T size)
{
	SUS_ASSERT(buff != NULL && source != NULL);
	if (buff > source && buff < source + size) for (SIZE_T i = size; i > 0; --i) buff[i - 1] = source[i - 1];
	else __movsb(buff, source, size);
}

//////////////////////////////////////////////////////////////////
//					Dynamic memory in heaps						//
//////////////////////////////////////////////////////////////////

// Allocate memory to the heap
SUS_LPMEMORY SUSAPI sus_malloc(
	_In_ SIZE_T size
);
// Allocating a memory array
SUS_LPMEMORY SUSAPI sus_calloc(
	_In_ DWORD count,
	_In_ SIZE_T size
);
// Memory reallocation
SUS_LPMEMORY SUSAPI sus_realloc(
	_In_ SUS_LPMEMORY block,
	_In_ SIZE_T newSize
);
	// Free a block of memory in the heap
SUS_LPMEMORY SUSAPI sus_free(
	_In_ SUS_LPMEMORY block
);

	// Fast memory allocation
#define sus_fmalloc(size) (SUS_LPMEMORY)HeapAlloc(GetProcessHeap(), 0, size)
// Fast allocating a memory array
#define sus_fcalloc(count, size) (SUS_LPMEMORY)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, size * count)
// Fast memory reallocation
#define sus_frealloc(block, newSize) (SUS_LPMEMORY)HeapReAlloc(GetProcessHeap(), 0, block, newSize)
// Releasing memory with pointer cleanup
#define sus_sfree(block) do { sus_free(block); block = NULL; } while (0)

//////////////////////////////////////////////////////////////////
//							Dynamic Data						//
//////////////////////////////////////////////////////////////////


typedef struct sus_data_view {
	LPBYTE data;
	SIZE_T size;
} SUS_DATAVIEW, *SUS_PDATAVIEW, *SUS_LPDATAVIEW;

// Create new dynamic memory
SUS_INLINE SUS_DATAVIEW SUSAPI susNewData(_In_ SIZE_T size) {
	SUS_ASSERT(size);
	return (SUS_DATAVIEW) {
		.data = sus_fmalloc(size),
		.size = size
	};
}
// Destroy dynamic data
SUS_INLINE VOID SUSAPI susDataDestroy(_In_ SUS_DATAVIEW data) {
	SUS_ASSERT(data.data);
	sus_free(data.data);
}

//////////////////////////////////////////////////////////////////
//					Dynamic virtual memory						//
//////////////////////////////////////////////////////////////////

// Allocate virtual memory
SUS_LPMEMORY SUSAPI sus_vmalloc(
	_In_opt_ SUS_LPMEMORY lpAddress,
	_In_ SIZE_T size,
	_In_ DWORD flAllocationType,
	_In_ DWORD flProtect
);
// Allocate virtual memory to process
SUS_LPMEMORY SUSAPI sus_vmallocEx(
	_In_ SUS_FILE hProcess,
	_In_opt_ SUS_LPMEMORY lpAddress,
	_In_ SIZE_T size,
	_In_ DWORD flAllocationType,
	_In_ DWORD flProtect
);

#define sus_vfree(block) VirtualFree(block, 0, MEM_RELEASE)

#ifdef __cplusplus
}
#endif // !__cplusplus

#endif /* !_SUS_MEMORY_ */
