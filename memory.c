// memory.c
//
#include "coreframe.h"
#include "include/susfwk/core.h"
#include "include/susfwk/memory.h"

//////////////////////////////////////////////////////////////////
//					Dynamic memory in heaps						//
//////////////////////////////////////////////////////////////////

// Allocate memory to the heap
SUS_LPMEMORY SUSAPI sus_malloc(_In_ SIZE_T size)
{
	SUS_PRINTDL("Allocating %d bytes of memory", size);
	SUS_LPMEMORY hMem = NULL;
	for (DWORD i = 0; i < SUS_NUMBER_ATTEMPTS_ALLOCATE_MEMORY; i++) {
		hMem = HeapAlloc(GetProcessHeap(), 0, size);
		if (hMem) break;
		SUS_PRINTDE("Couldn't allocate memory");
		SUS_PRINTDC(GetLastError());
		Sleep(10);
	}
	SUS_PRINTDL("Successful memory allocation");
	return hMem;
}
// Allocating a memory array
SUS_LPMEMORY SUSAPI sus_calloc(
	_In_ DWORD count,
	_In_ SIZE_T size)
{
	size = size * count;
	SUS_PRINTDL("Allocating a %d-byte memory array", size);
	if (size == 0) return NULL;
	SUS_LPMEMORY hMem = NULL;
	for (DWORD i = 0; i < SUS_NUMBER_ATTEMPTS_ALLOCATE_MEMORY; i++) {
		hMem = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, size);
		if (hMem) break;
		SUS_PRINTDE("Couldn't allocate memory");
		SUS_PRINTDC(GetLastError());
		Sleep(10);
	}
	SUS_PRINTDL("Successful memory allocation");
	return hMem;
}
// Memory reallocation
SUS_LPMEMORY SUSAPI sus_realloc(
	_In_ SUS_LPMEMORY block,
	_In_ SIZE_T newSize)
{
	SUS_PRINTDL("Changing the memory block to %d bytes", newSize);
	if (!block) return sus_malloc(newSize);
	SIZE_T oldSize = HeapSize(GetProcessHeap(), 0, block);
	if (oldSize == (SIZE_T)-1) return NULL;
	SUS_LPMEMORY hMem = HeapReAlloc(GetProcessHeap(), 0, block, newSize);
	if (hMem) {
		SUS_PRINTDL("The block size has been successfully changed");
		return hMem;
	}
	SUS_LPMEMORY hNewMem = sus_malloc(newSize);
	if (!hNewMem) return newSize < oldSize ? block : NULL;
	sus_memcpy(hNewMem, block, min(oldSize, newSize));
	sus_free(block);
	SUS_PRINTDL("The block size has been successfully changed");
	return hMem;
}
// Free a block of memory in the heap
SUS_LPMEMORY SUSAPI sus_free(_In_ SUS_LPMEMORY block)
{
	SUS_PRINTDL("Freeing the memory block");
	if (!HeapFree(GetProcessHeap(), 0, block)) {
		SUS_PRINTDE("The memory block could not be released");
		SUS_PRINTDC(GetLastError());
		return block;
	}
	SUS_PRINTDL("The memory block has been successfully released");
	return NULL;
}

//////////////////////////////////////////////////////////////////
//					Dynamic virtual memory						//
//////////////////////////////////////////////////////////////////

// Allocate virtual memory
SUS_LPMEMORY SUSAPI sus_vmalloc(
	_In_opt_ SUS_LPMEMORY lpAddress,
	_In_ SIZE_T size,
	_In_ DWORD flAllocationType,
	_In_ DWORD flProtect)
{
	SUS_PRINTDL("Allocating %d bytes in virtual memory", size);
	SUS_LPMEMORY hMem = VirtualAlloc(
		lpAddress,
		size,
		flAllocationType,
		flProtect
	);
	if (!hMem) {
		SUS_PRINTDE("Couldn't allocate memory");
		SUS_PRINTDC(GetLastError());
		return NULL;
	}
	SUS_PRINTDL("Virtual memory has been allocated successfully");
	return hMem;
}
// Allocate virtual memory to process
SUS_LPMEMORY SUSAPI sus_vmallocEx(
	_In_ SUS_FILE hProcess,
	_In_opt_ SUS_LPMEMORY lpAddress,
	_In_ SIZE_T size,
	_In_ DWORD flAllocationType,
	_In_ DWORD flProtect)
{
	SUS_PRINTDL("Allocating %d bytes in the process's virtual memory", size);
	SUS_LPMEMORY hMem = VirtualAllocEx(
		hProcess,
		lpAddress,
		size,
		flAllocationType,
		flProtect
	);
	if (!hMem) {
		SUS_PRINTDE("Couldn't allocate memory");
		SUS_PRINTDC(GetLastError());
		return NULL;
	}
	SUS_PRINTDL("Virtual memory has been allocated successfully");
	return hMem;
}