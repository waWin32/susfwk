// filemap.h
//
#ifndef _SUS_FILE_MAP_
#define _SUS_FILE_MAP_

// --------------------------------------------------------

// The structure of the file display in memory
typedef struct sus_file_map {
	SUS_FILE		hFile;	// The file descriptor
	SUS_FILE		hMap;	// The map descriptor
	SUS_DATAVIEW	data;	// Data
} SUS_FILE_MAP, *SUS_LPFILE_MAP;

// --------------------------------------------------------

// Open a file in memory
SUS_FILE_MAP SUSAPI susFileMapOpenA(
	_In_ LPCSTR fileName,
	_In_ DWORD dwAccess,
	_In_ SIZE_T offset,
	_In_ SIZE_T size
);
// Open a file in memory
SUS_FILE_MAP SUSAPI susFileMapOpenW(
	_In_ LPCWSTR fileName,
	_In_ DWORD dwAccess,
	_In_ SIZE_T offset,
	_In_ SIZE_T size
);

#ifdef UNICODE
#define susFileMapOpen	susFileMapOpenW
#else
#define susFileMapOpen	susFileMapOpenA
#endif // !UNICODE

// Close the file map
VOID SUSAPI susFileMapClose(
	_In_ SUS_FILE_MAP map
);
// Write a file map
BOOL SUSAPI susFileMapFlush(_In_ SUS_FILE_MAP map) {
	SUS_ASSERT(map.data.data && map.data.size);
	return FlushViewOfFile(map.data.data, map.data.size);
}
// --------------------------------------------------------

#endif /* !_SUS_FILE_MAP_ */