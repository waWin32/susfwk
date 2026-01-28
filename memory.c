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
#ifdef _DEBUG
	if (size >= (SIZE_T)1 << 20) SUS_PRINTDW("The allocated block exceeds the memory heap limits, use sus_vmalloc!");
#endif // ~_DEBUG
	SUS_LPMEMORY hMem = NULL;
	for (DWORD i = 0; i < SUS_NUMBER_ATTEMPTS_ALLOCATE_MEMORY; i++) {
		hMem = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, size);
		if (hMem) break;
		SUS_PRINTDE("Couldn't allocate memory");
		SUS_PRINTDC(GetLastError());
		susErrorPushEx((SUS_ERROR) { .sev = SUS_ERROR_SEVERITY_CRITICAL, .type = SUS_ERROR_TYPE_MEMORY, .code = SUS_ERROR_SYSTEM_ERROR });
		Sleep(10);
	}
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
	SUS_LPMEMORY hMem = HeapReAlloc(GetProcessHeap(), 0, block, newSize);
	if (hMem) {
		SUS_PRINTDL("The block size has been successfully changed");
		return hMem;
	}
	SUS_LPMEMORY hNewMem = sus_malloc(newSize);
	if (!hNewMem) return newSize <= oldSize ? block : NULL;
	sus_memcpy(hNewMem, block, min(oldSize, newSize));
	sus_free(block);
	return hNewMem;
}
// Free a block of memory in the heap
SUS_LPMEMORY SUSAPI sus_free(_In_ SUS_LPMEMORY block)
{
	SUS_PRINTDL("Freeing the memory block");
	if (!HeapFree(GetProcessHeap(), 0, block)) {
		SUS_PRINTDE("The memory block could not be released");
		SUS_PRINTDC(GetLastError());
		susErrorPushEx((SUS_ERROR) { .sev = SUS_ERROR_SEVERITY_CRITICAL, .type = SUS_ERROR_TYPE_MEMORY, .code = SUS_ERROR_SYSTEM_ERROR });
		return block;
	}
	return NULL;
}
// Create and initialize memory
SUS_LPMEMORY SUSAPI sus_newmem(_In_ SIZE_T size, _In_opt_ SUS_OBJECT value)
{
	SUS_ASSERT(size);
	SUS_LPMEMORY obj = sus_malloc(size);
	if (value) sus_memcpy(obj, (sus_lpbyte_t)value, size);
	else sus_zeromem(obj, size);
	return obj;
}

//////////////////////////////////////////////////////////////////
//					Dynamic virtual memory						//
//////////////////////////////////////////////////////////////////

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
		susErrorPushEx((SUS_ERROR) { .sev = SUS_ERROR_SEVERITY_CRITICAL, .type = SUS_ERROR_TYPE_MEMORY, .code = SUS_ERROR_SYSTEM_ERROR });
		return NULL;
	}
	return hMem;
}
// Allocate virtual memory
SUS_LPMEMORY SUSAPI sus_vmalloc(
	_In_ SIZE_T size,
	_In_ SUS_MEMORY_PROTECT protect)
{
	SUS_PRINTDL("Allocating %d bytes in virtual memory", size);
	SUS_LPMEMORY hMem = VirtualAlloc(
		NULL,
		size,
		MEM_RESERVE | MEM_COMMIT,
		protect
	);
	if (!hMem) {
		SUS_PRINTDE("Couldn't allocate memory");
		SUS_PRINTDC(GetLastError());
		susErrorPushEx((SUS_ERROR) { .sev = SUS_ERROR_SEVERITY_CRITICAL, .type = SUS_ERROR_TYPE_MEMORY, .code = SUS_ERROR_SYSTEM_ERROR });
		return NULL;
	}
	return hMem;
}