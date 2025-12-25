// filemap.c
//
#include "coreframe.h"
#include "include/susfwk/core.h"
#include "include/susfwk/vector.h"
#include "include/susfwk/filemap.h"

// --------------------------------------------------------

static SUS_DATAVIEW SUSAPI susFileMapView(_In_ SUS_FILE hMap, _In_ SIZE_T offset, _In_ SIZE_T size, _In_ DWORD dwFileAccess)
{
	SUS_PRINTDL("Displaying a file in memory");
	SUS_ASSERT(hMap);
	SUS_DATAVIEW data = { 0 };
	data.data = MapViewOfFile(hMap, (dwFileAccess & GENERIC_WRITE ? FILE_MAP_ALL_ACCESS : FILE_MAP_READ), (DWORD)(offset >> 32), (DWORD)(offset & 0xFFFFFFFF), size);
	if (!data.data) {
		SUS_PRINTDE("Failed to display the file in memory");
		return data;
	}
	data.size = size;	
	return data;
}

// Open a file in memory
SUS_FILE_MAP SUSAPI susFileMapOpenA(_In_ LPCSTR fileName, _In_ DWORD dwAccess, _In_ SIZE_T offset, _In_ SIZE_T size)
{
	SUS_PRINTDL("Opening a file in memory", fileName);
	SUS_ASSERT(fileName);
	SUS_FILE_MAP map = { 0 };
	BOOL fileOwner = !sus_fexistsA(fileName);
	map.hFile = sus_fopenA(fileName, dwAccess);
	if (!map.hFile) {
		SUS_PRINTDE("Couldn't open the file in memory");
		return (SUS_FILE_MAP) { 0 };
	}
	size = size ? size : sus_fsize(map.hFile);
	map.hMap = CreateFileMappingA(map.hFile, NULL, (dwAccess & GENERIC_WRITE ? PAGE_READWRITE : PAGE_READONLY), (DWORD)((offset + size) >> 32), (DWORD)((offset + size) & 0xFFFFFFFF), NULL);
	if (!map.hMap) {
		SUS_PRINTDE("Couldn't open the file in memory");
		if (fileOwner) sus_fremoveA(fileName);
		sus_fclose(map.hFile);
		return (SUS_FILE_MAP) { 0 };
	}
	map.data = susFileMapView(map.hMap, offset, size, dwAccess);
	if (!map.data.data) {
		SUS_PRINTDE("Couldn't open the file in memory");
		sus_fclose(map.hMap);
		if (fileOwner) sus_fremoveA(fileName);
		sus_fclose(map.hFile);
		return (SUS_FILE_MAP) { 0 };
	}
	SUS_PRINTDL("The file is successfully opened in memory");
	return map;
}
// Open a file in memory
SUS_FILE_MAP SUSAPI susFileMapOpenW(_In_ LPCWSTR fileName, _In_ DWORD dwAccess, _In_ SIZE_T offset, _In_ SIZE_T size)
{
	SUS_PRINTDL("Opening a file in memory", fileName);
	SUS_ASSERT(fileName);
	SUS_FILE_MAP map = { 0 };
	BOOL fileOwner = !sus_fexistsW(fileName);
	map.hFile = sus_fopenW(fileName, dwAccess);
	if (!map.hFile) {
		SUS_PRINTDE("Couldn't open the file in memory");
		return (SUS_FILE_MAP) { 0 };
	}
	size = size ? size : sus_fsize(map.hFile);
	map.hMap = CreateFileMappingW(map.hFile, NULL, (dwAccess & GENERIC_WRITE ? PAGE_READWRITE : PAGE_READONLY), (DWORD)((offset + size) >> 32), (DWORD)((offset + size) & 0xFFFFFFFF), NULL);
	if (!map.hMap) {
		SUS_PRINTDE("Couldn't open the file in memory");
		if (fileOwner) sus_fremoveW(fileName);
		sus_fclose(map.hFile);
		return (SUS_FILE_MAP) { 0 };
	}
	map.data = susFileMapView(map.hMap, offset, size, dwAccess);
	if (!map.data.data) {
		SUS_PRINTDE("Couldn't open the file in memory");
		sus_fclose(map.hMap);
		if (fileOwner) sus_fremoveW(fileName);
		sus_fclose(map.hFile);
		return (SUS_FILE_MAP) { 0 };
	}
	SUS_PRINTDL("The file is successfully opened in memory");
	return map;
}

// Close the file map
VOID SUSAPI susFileMapClose(_In_ SUS_FILE_MAP map)
{
	SUS_PRINTDL("Closing the file map");
	SUS_ASSERT(map.hFile && map.hMap);
	UnmapViewOfFile(map.data.data);
	sus_fclose(map.hMap);
	sus_fclose(map.hFile);
}

// --------------------------------------------------------
