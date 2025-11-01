// fileio.h
//
#ifndef _SUS_FILE_IO_
#define _SUS_FILE_IO_

// --------------------------------------------------------

// Opening a file
SUS_FILE SUSAPI sus_fopenexA(
	_In_ LPCSTR lpFileName,
	_In_ DWORD dwDesiredAccess,
	_In_ DWORD dwShareMode,
	_In_ DWORD dwCreationDisposition,
	_In_ DWORD dwFlagsAndAttributes
);
// Creating a file
SUS_FILE SUSAPI sus_fopenexW(
	_In_ LPCWSTR lpFileName,
	_In_ DWORD dwDesiredAccess,
	_In_ DWORD dwShareMode,
	_In_ DWORD dwCreationDisposition,
	_In_ DWORD dwFlagsAndAttributes
);

#ifdef UNICODE
#define sus_fopenex	sus_fopenexW
#else
#define sus_fopenex	sus_fopenexA
#endif // !UNICODE

// Opening a file
#define sus_fopenA(fileName, access) sus_fopenexA(fileName, access, FILE_SHARE_READ, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL)
// Opening a file
#define sus_fopenW(fileName, access) sus_fopenexW(fileName, access, FILE_SHARE_READ, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL)
// Creating a new file
#define sus_fcreateA(fileName, attributes) sus_fopenexA(fileName, GENERIC_WRITE, 0, CREATE_ALWAYS, attributes)
// Creating a new file
#define sus_fcreateW(fileName, attributes) sus_fopenexW(fileName, GENERIC_WRITE, 0, CREATE_ALWAYS, attributes)

#ifdef UNICODE
#define sus_fopen			sus_fopenW
#define sus_fcreate			sus_fcreateW
#else
#define sus_fopen			sus_fopenA
#define sus_fcreate			sus_fcreateA
#endif // !UNICODE

// Create a folder
BOOL SUSAPI sus_mkdirA(_In_ LPCSTR path);
// Create a folder
BOOL SUSAPI sus_mkdirW(_In_ LPCWSTR path);

#ifdef UNICODE
#define sus_mkdir	sus_mkdirW
#else
#define sus_mkdir	sus_mkdirA
#endif // !UNICODE

// --------------------------------------------------------

// Delete a file
BOOL SUSAPI sus_fremoveA(
	_In_ LPCSTR lpFileName
);
// Delete a file
BOOL SUSAPI sus_fremoveW(
	_In_ LPCWSTR lpFileName
);

#ifdef UNICODE
#define sus_fremove	sus_fremoveW
#else
#define sus_fremove	sus_fremoveA
#endif // !UNICODE

// --------------------------------------------------------

// Flag - delete the file after closing (No file deletion required)
#define SUS_TEMP_FILE_DELETE_ON_CLOSE		FILE_FLAG_DELETE_ON_CLOSE
// Flag - optimize for random access
#define SUS_TEMP_FILE_RANDOM_ACCESS			FILE_FLAG_RANDOM_ACCESS
// Flag - optimize for sequential read/write
#define SUS_TEMP_FILE_SEQUENTIAL_SCAN		FILE_FLAG_SEQUENTIAL_SCAN
// The flag is to disable hashing
#define SUS_TEMP_FILE_DISABLE_BUFFERING		FILE_FLAG_NO_BUFFERING
// Flag - synchronous recording to disk
#define SUS_TEMP_FILE_WRITE_THROUGH			FILE_FLAG_WRITE_THROUGH
// The file attribute is hidden
#define SUS_TEMP_FILE_HIDDEN				FILE_ATTRIBUTE_HIDDEN

// Create a temporary file
SUS_FILE SUSAPI susCreateTempFileA(
	_In_opt_ LPCSTR lpPrefixString,
	_In_opt_ LPCSTR lpFileExtension,
	_In_ sus_flag32 flags,
	_Out_writes_opt_(MAX_PATH) LPSTR lpTempFileName
);
// Create a temporary file
SUS_FILE SUSAPI susCreateTempFileW(
	_In_opt_ LPCWSTR lpPrefixString,
	_In_opt_ LPCWSTR lpFileExtension,
	_In_ sus_flag32 flags,
	_Out_writes_opt_(MAX_PATH) LPWSTR lpTempFileName
);

#ifdef UNICODE
#define susCreateTempFile	susCreateTempFileW
#else
#define susCreateTempFile	susCreateTempFileA
#endif // !UNICODE

// --------------------------------------------------------

// Reading data from a file
INT SUSAPI sus_freadex(
	_In_ SUS_FILE hFile,
	_Out_ LPBYTE lpBuffer,
	_In_ DWORD dwReadBufferSize
);
// Read the entire file
SUS_DATAVIEW SUSAPI sus_fread(_In_ SUS_FILE hFile);

// Writing to a file
DWORD SUSAPI sus_fwrite(
	_In_ SUS_FILE hFile,
	_In_ CONST LPBYTE lpData,
	_In_ DWORD dwNumberOfBytesWrite
);
// Move the pointer to the specified blend
#define sus_fseek(hFile, offset, origin)	SetFilePointerEx(hFile, (LARGE_INTEGER) { .QuadPart = offset }, NULL, origin)

// --------------------------------------------------------

// Get the file size
LONGLONG SUSAPI sus_fsize(
	_In_ SUS_FILE hFile
);

typedef struct sus_file_stat {
	DWORD dwFileAttributes;
	FILETIME ftCreationTime;
	FILETIME ftLastAccessTime;
	FILETIME ftLastWriteTime;
	ULONGLONG dwFileSize;
} SUS_FSTAT, *SUS_PFSTAT, *SUS_LPFSTAT;

// Get file statistics
SUS_FSTAT SUSAPI sus_fstateA(_In_ LPCSTR lpFileName);
// Get file statistics
SUS_FSTAT SUSAPI sus_fstateW(_In_ LPCWSTR lpFileName);

#ifdef UNICODE
#define sus_fstate				sus_fstateW
#else
#define sus_fstate				sus_fstateA
#endif // !UNICODE

// --------------------------------------------------------

// File Search Handler
typedef DWORD(SUSAPI* SUS_FILE_SEARCH_PROCESSORA)(LPSTR path, _In_opt_ SUS_OBJECT userData);
// Recursive traversal of hard disk files
DWORD SUSAPI susTraverseFileTreeA(
	_In_ LPCSTR directory,
	_In_ SUS_FILE_SEARCH_PROCESSORA lpFileSearchProc,
	_In_opt_ SUS_OBJECT userData
);
// Navigate through the folders and files in directory
BOOL SUSAPI susEnumDirectoryA(
	_In_ LPCSTR directory,
	_In_ SUS_FILE_SEARCH_PROCESSORA lpFileSearchProc,
	_In_opt_ SUS_OBJECT userData
);

// File Search Handler
typedef DWORD(SUSAPI* SUS_FILE_SEARCH_PROCESSORW)(LPWSTR path, _In_opt_ SUS_OBJECT userData);
// Recursive traversal of hard disk files
DWORD SUSAPI susTraverseFileTreeW(
	_In_ LPCWSTR directory,
	_In_ SUS_FILE_SEARCH_PROCESSORW lpFileSearchProc,
	_In_opt_ SUS_OBJECT userData
);
// Navigate through the folders and files in directory
BOOL SUSAPI susEnumDirectoryW(
	_In_ LPCWSTR directory,
	_In_ SUS_FILE_SEARCH_PROCESSORW lpFileSearchProc,
	_In_opt_ SUS_OBJECT userData
);

#ifdef UNICODE
#define susTraverseFileTree	susTraverseFileTreeW
#define susEnumDirectory	susEnumDirectoryW
#else
#define susTraverseFileTree	susTraverseFileTreeA
#define susEnumDirectory	susEnumDirectoryA
#endif // !UNICODE

#endif /* !_SUS_FILE_IO_ */
