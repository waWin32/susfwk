// fileio.h
//
#ifndef _SUS_FILE_IO_
#define _SUS_FILE_IO_

// Opening a file
SUS_FILE SUSAPI susCreateFileA(
	_In_ LPCSTR lpFileName,
	_In_ DWORD DesiredAccess,
	_In_ DWORD ShareMode,
	_In_ DWORD CreationDisposition,
	_In_ DWORD FlagsAndAttributes
);
// Creating a file
SUS_FILE SUSAPI susCreateFileW(
	_In_ LPCWSTR lpFileName,
	_In_ DWORD dwDesiredAccess,
	_In_ DWORD dwShareMode,
	_In_ DWORD dwCreationDisposition,
	_In_ DWORD dwFlagsAndAttributes
);

#ifdef UNICODE
#define susCreateFile	susCreateFileW
#else
#define susCreateFile	susCreateFileA
#endif // !UNICODE

// Opening a file
#define sus_fopenA(fileName, access) susCreateFileA(fileName, access, FILE_SHARE_READ, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL)
// Opening a file
#define sus_fopenW(fileName, access) susCreateFileW(fileName, access, FILE_SHARE_READ, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL)
// Creating a new file
#define sus_fcreateA(fileName, attributes) susCreateFileA(fileName, GENERIC_WRITE, 0, CREATE_ALWAYS, attributes)
// Creating a new file
#define sus_fcreateW(fileName, attributes) susCreateFileW(fileName, GENERIC_WRITE, 0, CREATE_ALWAYS, attributes)

#ifdef UNICODE
#define sus_fopen			sus_fopenW
#define sus_fcreate			sus_fcreateW
#else
#define sus_fopen			sus_fopenA
#define sus_fcreate			sus_fcreateA
#endif // !UNICODE

// Delete a file
BOOL SUSAPI susDeleteFileA(
	_In_ LPCSTR lpFileName
);
// Delete a file
BOOL SUSAPI susDeleteFileW(
	_In_ LPCWSTR lpFileName
);

#ifdef UNICODE
#define susDeleteFile	susDeleteFileW
#else
#define susDeleteFile	susDeleteFileA
#endif // !UNICODE

// Flag - delete the file after closing (No file deletion required)
#define SUS_TEMP_FILE_DELETE_ON_CLOSE		1 << 0
// Flag - optimize for random access
#define SUS_TEMP_FILE_RANDOM_ACCESS			1 << 1
// Flag - optimize for sequential read/write
#define SUS_TEMP_FILE_SEQUENTIAL_SCAN		1 << 2
// The flag is to disable hashing
#define SUS_TEMP_FILE_DISABLE_BUFFERING		1 << 3
// Flag - synchronous recording to disk
#define SUS_TEMP_FILE_WRITE_THROUGH			1 << 4
// The file attribute is hidden
#define SUS_TEMP_FILE_HIDDEN				1 << 5

// Create a temporary file
SUS_FILE SUSAPI susCreateTempFileA(
	_In_opt_ LPCSTR lpPrefixString,
	_In_opt_ LPCSTR lpFileExtension,
	_In_ SUS_FLAG16 flags,
	_Out_writes_opt_(MAX_PATH) LPSTR lpTempFileName
);
// Create a temporary file
SUS_FILE SUSAPI susCreateTempFileW(
	_In_opt_ LPCWSTR lpPrefixString,
	_In_opt_ LPCWSTR lpFileExtension,
	_In_ SUS_FLAG16 flags,
	_Out_writes_opt_(MAX_PATH) LPWSTR lpTempFileName
);

#ifdef UNICODE
#define susCreateTempFile	susCreateTempFileW
#else
#define susCreateTempFile	susCreateTempFileA
#endif // !UNICODE

// --------------------------------------------------------

// Reading data from a file
DWORD SUSAPI susReadFile(
	_In_ SUS_FILE hFile,
	_Out_ LPBYTE lpBuffer,
	_In_ DWORD dwReadBufferSize
);

// Reading data from a file
#define sus_fread(file, lpBuffer, dwReadBufferSize)	susReadFile(file, lpBuffer, dwReadBufferSize)

// Writing to a file
DWORD SUSAPI susWriteFile(
	_In_ SUS_FILE hFile,
	_In_ CONST LPBYTE lpData,
	_In_ DWORD dwNumberOfBytesWrite
);

// Writing to a file
#define sus_fwrite(file, lpData, dwNumberOfBytesWrite)	susWriteFile(file, lpData, dwNumberOfBytesWrite <= 0 ? lstrlenA(lpData) : dwNumberOfBytesWrite)

// Get the file size
LONGLONG SUSAPI susGetFileSize(
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
SUS_FSTAT SUSAPI susGetFileAttributesA(_In_ LPCSTR lpFileName);
// Get file statistics
SUS_FSTAT SUSAPI susGetFileAttributesW(_In_ LPCWSTR lpFileName);

// Get file statistics
#define sus_fstateA(fileName)	susGetFileAttributesA(fileName)
// Get file statistics
#define sus_fstateW(fileName)	susGetFileAttributesW(fileName)

#ifdef UNICODE
#define susGetFileAttributes	susGetFileAttributesW
#define sus_fstate				sus_fstateW
#else
#define susGetFileAttributes	susGetFileAttributesA
#define sus_fstate				sus_fstateA
#endif // !UNICODE

// Move the pointer to the specified blend
#define sus_fseek(hFile, offset, origin)	SetFilePointerEx(hFile, (LARGE_INTEGER) { .QuadPart = offset }, NULL origin)

// --------------------------------------------------------

// File Search Handler
typedef DWORD(SUSAPI* SUS_FILE_SEARCH_PROCESSORA)(LPSTR path);
// Recursive traversal of hard disk files
DWORD SUSAPI susDirectorySearchA(
	_In_ LPSTR directory,
	_In_ SUS_FILE_SEARCH_PROCESSORA lpFileSearchProc
);

// File Search Handler
typedef DWORD(SUSAPI* SUS_FILE_SEARCH_PROCESSORW)(LPWSTR path);
// Recursive traversal of hard disk files
DWORD SUSAPI susDirectorySearchW(
	_In_ LPWSTR directory,
	_In_ SUS_FILE_SEARCH_PROCESSORW lpFileSearchProc
);

#endif /* !_SUS_FILE_IO_ */
