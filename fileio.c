// conio.c
//
#include "coreframe.h"
#include "include/susfwk/core.h"
#include "include/susfwk/string.h"
#include "include/susfwk/fileio.h"

// Opening a file
SUS_FILE SUSAPI susCreateFileA(
	_In_ LPCSTR lpFileName,
	_In_ DWORD dwDesiredAccess,
	_In_ DWORD dwShareMode,
	_In_ DWORD dwCreationDisposition,
	_In_ DWORD dwFlagsAndAttributes)
{
	SUS_PRINTDL("Opening a file");
	SUS_ASSERT(lpFileName);
	SUS_FILE hFile = CreateFileA(
		lpFileName,
		dwDesiredAccess,
		dwShareMode,
		NULL,
		dwCreationDisposition,
		dwFlagsAndAttributes,
		NULL
	);
	if (hFile == INVALID_HANDLE_VALUE) {
		SUS_PRINTDE("Error when opening the file");
		SUS_PRINTDC(GetLastError());
		return NULL;
	}
	SUS_PRINTDL("The file has been opened successfully");
	return hFile;
}
// Creating a file
SUS_FILE SUSAPI susCreateFileW(
	_In_ LPCWSTR lpFileName,
	_In_ DWORD dwDesiredAccess,
	_In_ DWORD dwShareMode,
	_In_ DWORD dwCreationDisposition,
	_In_ DWORD dwFlagsAndAttributes)
{
	SUS_PRINTDL("Creating a file");
	SUS_ASSERT(lpFileName);
	SUS_FILE hFile = CreateFileW(
		lpFileName,
		dwDesiredAccess,
		dwShareMode,
		NULL,
		dwCreationDisposition,
		dwFlagsAndAttributes,
		NULL
	);
	if (hFile == INVALID_HANDLE_VALUE) {
		SUS_PRINTDE("Error when creating the file");
		SUS_PRINTDC(GetLastError());
		return NULL;
	}
	SUS_PRINTDL("The file has been opened successfully");
	return hFile;
}
// Delete a file
BOOL SUSAPI susDeleteFileA(_In_ LPCSTR lpFileName)
{
	SUS_PRINTDL("Deleting a file");
	SUS_ASSERT(lpFileName);
	if (!DeleteFileA(lpFileName)) {
		SUS_PRINTDL("Couldn't delete the file");
		SUS_PRINTDC(GetLastError());
		return FALSE;
	}
	SUS_PRINTDL("The file was deleted successfully");
	return TRUE;
}
// Delete a file
BOOL SUSAPI susDeleteFileW(_In_ LPCWSTR lpFileName)
{
	SUS_PRINTDL("Deleting a file");
	SUS_ASSERT(lpFileName);
	if (!DeleteFileW(lpFileName)) {
		SUS_PRINTDL("Couldn't delete the file");
		SUS_PRINTDC(GetLastError());
		return FALSE;
	}
	SUS_PRINTDL("The file was deleted successfully");
	return TRUE;
}

// Create a temporary file
SUS_FILE SUSAPI susCreateTempFileA(
	_In_opt_ LPCSTR lpPrefixString,
	_In_opt_ LPCSTR lpFileExtension,
	_In_ SUS_FLAG16 flags,
	_Out_writes_opt_(MAX_PATH) LPSTR lpTempFileName)
{
	SUS_PRINTDL("Creating a temporary file");
	CHAR tempDir[MAX_PATH];
	if (lpTempFileName) *lpTempFileName = '\0';
	if (GetTempPathA(MAX_PATH, tempDir) == 0) {
		SUS_PRINTDE("Couldn't create a temporary file");
		SUS_PRINTDC(GetLastError());
		return NULL;
	}
	CHAR _tempFileName[MAX_PATH];
	if (GetTempFileNameA(
		tempDir,
		lpPrefixString ? lpPrefixString : "tmp",
		0,
		_tempFileName
	) == 0) {
		SUS_PRINTDE("Couldn't create a temporary file");
		SUS_PRINTDC(GetLastError());
		return NULL;
	}
	susDeleteFileA(_tempFileName);
	if (lpFileExtension) {
		LPSTR cur = sus_strrchrA(_tempFileName, '.');
		if (cur) lstrcpyA(cur, lpFileExtension);
	}
	SUS_FILE hFile = susCreateFileA(
		_tempFileName, GENERIC_READ | GENERIC_WRITE,
		FILE_SHARE_READ | FILE_SHARE_DELETE, CREATE_ALWAYS,
		FILE_ATTRIBUTE_TEMPORARY | 
		(flags & SUS_TEMP_FILE_DELETE_ON_CLOSE ? FILE_FLAG_DELETE_ON_CLOSE : 0) |
		(flags & SUS_TEMP_FILE_RANDOM_ACCESS ? FILE_FLAG_RANDOM_ACCESS : 0) |
		(flags & SUS_TEMP_FILE_SEQUENTIAL_SCAN ? FILE_FLAG_SEQUENTIAL_SCAN : 0) |
		(flags & SUS_TEMP_FILE_DISABLE_BUFFERING ? FILE_FLAG_NO_BUFFERING : 0) |
		(flags & SUS_TEMP_FILE_HIDDEN ? FILE_ATTRIBUTE_HIDDEN : 0) |
		(flags & SUS_TEMP_FILE_WRITE_THROUGH ? FILE_FLAG_WRITE_THROUGH : 0)
	);
	if (!hFile) {
		SUS_PRINTDE("Couldn't create a temporary file");
		SUS_PRINTDC(GetLastError());
		return NULL;
	}
	if (lpTempFileName) lstrcpyA(lpTempFileName, _tempFileName);
	SUS_PRINTDL("The temporary file has been created successfully!");
	return hFile;
}
// Create a temporary file
SUS_FILE SUSAPI susCreateTempFileW(
	_In_opt_ LPCWSTR lpPrefixString,
	_In_opt_ LPCWSTR lpFileExtension,
	_In_ SUS_FLAG16 flags,
	_Out_writes_opt_(MAX_PATH) LPWSTR lpTempFileName)
{
	SUS_PRINTDL("Creating a temporary file");
	WCHAR tempDir[MAX_PATH];
	if (lpTempFileName) *lpTempFileName = L'\0';
	if (GetTempPathW(MAX_PATH, tempDir) == 0) {
		SUS_PRINTDE("Couldn't create a temporary file");
		SUS_PRINTDC(GetLastError());
		return NULL;
	}
	WCHAR _tempFileName[MAX_PATH];
	if (GetTempFileNameW(
		tempDir,
		lpPrefixString ? lpPrefixString : L"tmp",
		0,
		_tempFileName
	) == 0) {
		SUS_PRINTDE("Couldn't create a temporary file");
		SUS_PRINTDC(GetLastError());
		return NULL;
	}
	susDeleteFileW(_tempFileName);
	if (lpFileExtension) {
		LPWSTR cur = sus_strrchrW(_tempFileName, L'.');
		if (cur) lstrcpyW(cur, lpFileExtension);
	}
	SUS_FILE hFile = susCreateFileW(
		_tempFileName, GENERIC_READ | GENERIC_WRITE,
		FILE_SHARE_READ | FILE_SHARE_DELETE, CREATE_ALWAYS,
		FILE_ATTRIBUTE_TEMPORARY |
		(flags & SUS_TEMP_FILE_DELETE_ON_CLOSE ? FILE_FLAG_DELETE_ON_CLOSE : 0) |
		(flags & SUS_TEMP_FILE_RANDOM_ACCESS ? FILE_FLAG_RANDOM_ACCESS : 0) |
		(flags & SUS_TEMP_FILE_SEQUENTIAL_SCAN ? FILE_FLAG_SEQUENTIAL_SCAN : 0) |
		(flags & SUS_TEMP_FILE_DISABLE_BUFFERING ? FILE_FLAG_NO_BUFFERING : 0) |
		(flags & SUS_TEMP_FILE_HIDDEN ? FILE_ATTRIBUTE_HIDDEN : 0) |
		(flags & SUS_TEMP_FILE_WRITE_THROUGH ? FILE_FLAG_WRITE_THROUGH : 0)
	);
	if (!hFile) {
		SUS_PRINTDE("Couldn't create a temporary file");
		SUS_PRINTDC(GetLastError());
		return NULL;
	}
	if (lpTempFileName) lstrcpyW(lpTempFileName, _tempFileName);
	SUS_PRINTDL("The temporary file has been created successfully!");
	return hFile;
}

// --------------------------------------------------------

// Reading data from a file
DWORD SUSAPI susReadFile(
	_In_ SUS_FILE hFile,
	_Out_ LPBYTE lpBuffer,
	_In_ DWORD dwReadBufferSize)
{
	SUS_PRINTDL("Reading data from a file");
	SUS_ASSERT(lpBuffer && hFile);
	DWORD bytesRead;
	if (!ReadFile(
		hFile,
		lpBuffer,
		dwReadBufferSize,
		&bytesRead,
		NULL))
	{
		SUS_PRINTDE("Couldn't read data from the file");
		SUS_PRINTDC(GetLastError());
		return 0;
	}
	SUS_PRINTDL("The data has been read successfully");
	return bytesRead;
}
// Writing to a file
DWORD SUSAPI susWriteFile(
	_In_ SUS_FILE hFile,
	_In_ CONST LPBYTE lpData,
	_In_ DWORD dwNumberOfBytesWrite)
{
	SUS_PRINTDL("Writing to a file");
	SUS_ASSERT(lpData && hFile);
	DWORD bytesWritten;
	if (!WriteFile(
		hFile,
		lpData,
		dwNumberOfBytesWrite,
		&bytesWritten,
		NULL))
	{
		SUS_PRINTDE("Couldn't write data to a file");
		SUS_PRINTDC(GetLastError());
		return 0;
	}
	SUS_PRINTDL("The data has been successfully written to the file");
	return bytesWritten;
}

// Get the file size
LONGLONG SUSAPI susGetFileSize(_In_ SUS_FILE hFile)
{
	SUS_PRINTDL("Getting the file size");
	SUS_ASSERT(hFile);
	LARGE_INTEGER size;
	if (!GetFileSizeEx(hFile, &size)) {
		SUS_PRINTDE("Couldn't get the file size");
		return 0;
	}
	SUS_PRINTDL("File size successfully received: %d", size.QuadPart);
	return size.QuadPart;
}

// Get file statistics
SUS_FSTAT SUSAPI susGetFileAttributesA(_In_ LPCSTR lpFileName)
{
	SUS_PRINTDL("Getting file attributes");
	WIN32_FILE_ATTRIBUTE_DATA attrData;
	if (!GetFileAttributesExA(lpFileName, GetFileExInfoStandard, &attrData)) {
		SUS_PRINTDE("Couldn't get file attributes");
		SUS_PRINTDC(GetLastError());
		return (SUS_FSTAT) { 0 };
	}
	return (SUS_FSTAT) {
		.dwFileAttributes = attrData.dwFileAttributes,
		.ftCreationTime = attrData.ftCreationTime,
		.ftLastAccessTime = attrData.ftLastAccessTime,
		.ftLastWriteTime = attrData.ftLastWriteTime,
		.dwFileSize = (ULONGLONG)(((ULONGLONG)attrData.nFileSizeHigh << 32) | attrData.nFileSizeLow)
	};
}
// Get file statistics
SUS_FSTAT SUSAPI susGetFileAttributesW(_In_ LPCWSTR lpFileName)
{
	SUS_PRINTDL("Getting file attributes");
	WIN32_FILE_ATTRIBUTE_DATA attrData;
	if (!GetFileAttributesExW(lpFileName, GetFileExInfoStandard, &attrData)) {
		SUS_PRINTDE("Couldn't get file attributes");
		SUS_PRINTDC(GetLastError());
		return (SUS_FSTAT) { 0 };
	}
	return (SUS_FSTAT) {
		.dwFileAttributes = attrData.dwFileAttributes,
		.ftCreationTime = attrData.ftCreationTime,
		.ftLastAccessTime = attrData.ftLastAccessTime,
		.ftLastWriteTime = attrData.ftLastWriteTime,
		.dwFileSize = (ULONGLONG)(((ULONGLONG)attrData.nFileSizeHigh << 32) | attrData.nFileSizeLow)
	};
}


// --------------------------------------------------------

// Recursive traversal of hard disk files
DWORD SUSAPI susDirectorySearchA(
	_In_ LPSTR directory,
	_In_ SUS_FILE_SEARCH_PROCESSORA lpFileSearchProc)
{
	SUS_ASSERT(directory && lpFileSearchProc);
	PWIN32_FIND_DATAA lpFindFileData = sus_fmalloc(sizeof(WIN32_FIND_DATAA));
	if (!lpFindFileData) return 1;
	LPSTR searchPath = sus_fcalloc(sus_strlenA(directory) + 3, sizeof(CHAR));
	if (!searchPath) {
		sus_free(lpFindFileData);
		return 1;
	}
	lstrcpyA(searchPath, directory);
	lstrcatA(searchPath, "\\*");
	SUS_FILE hFind = FindFirstFileA(searchPath, lpFindFileData);
	sus_free(searchPath);
	if (hFind == INVALID_HANDLE_VALUE) {
		SUS_PRINTDE("Couldn't get the first file");
		SUS_PRINTDC(GetLastError());
		sus_free(lpFindFileData);
		return 2;
	}
	do {
		if (lstrcmpA(lpFindFileData->cFileName, ".") == 0 || lstrcmpA(lpFindFileData->cFileName, "..") == 0) continue;
		LPSTR filePath = sus_fcalloc(sus_strlenA(directory) + sus_strlenA(lpFindFileData->cFileName) + 3, sizeof(CHAR));
		if (!filePath) continue;
		lstrcpyA(filePath, directory);
		lstrcatA(filePath, "\\");
		lstrcatA(filePath, lpFindFileData->cFileName);
		if (lpFindFileData->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
			if (susDirectorySearchA(filePath, lpFileSearchProc) == GETTING_OUT_RECURSION) {
				FindClose(hFind);
				sus_free(filePath);
				sus_free(lpFindFileData);
				return GETTING_OUT_RECURSION;
			}
		}
		else {
			if (lpFileSearchProc(filePath) == GETTING_OUT_RECURSION) {
				SUS_PRINTDL("Completion of recursion");
				FindClose(hFind);
				sus_free(filePath);
				sus_free(lpFindFileData);
				return GETTING_OUT_RECURSION;
			}
		}
		sus_free(filePath);
	} while (FindNextFileA(hFind, lpFindFileData) != 0);
	sus_free(lpFindFileData);
	FindClose(hFind);
	return 0;
}
// Recursive traversal of hard disk files
DWORD SUSAPI susDirectorySearchW(
	_In_ LPWSTR directory,
	_In_ SUS_FILE_SEARCH_PROCESSORW lpFileSearchProc)
{
	SUS_ASSERT(directory && lpFileSearchProc);
	PWIN32_FIND_DATAW lpFindFileData = sus_fmalloc(sizeof(WIN32_FIND_DATAW));
	if (!lpFindFileData) return 1;
	LPWSTR searchPath = sus_fcalloc(sus_strlenW(directory) + 3, sizeof(WCHAR));
	if (!searchPath) {
		sus_free(lpFindFileData);
		return 1;
	}
	lstrcpyW(searchPath, directory);
	lstrcatW(searchPath, L"\\*");
	SUS_FILE hFind = FindFirstFileW(searchPath, lpFindFileData);
	sus_free(searchPath);
	if (hFind == INVALID_HANDLE_VALUE) {
		SUS_PRINTDE("Couldn't get the first file");
		SUS_PRINTDC(GetLastError());
		sus_free(lpFindFileData);
		return 2;
	}
	do {
		if (lstrcmpW(lpFindFileData->cFileName, L".") == 0 || lstrcmpW(lpFindFileData->cFileName, L"..") == 0) continue;
		LPWSTR filePath = sus_fcalloc(sus_strlenW(directory) + sus_strlenW(lpFindFileData->cFileName) + 3, sizeof(WCHAR));
		if (!filePath) continue;
		lstrcpyW(filePath, directory);
		lstrcatW(filePath, L"\\");
		lstrcatW(filePath, lpFindFileData->cFileName);
		if (lpFindFileData->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
			if (susDirectorySearchW(filePath, lpFileSearchProc) == GETTING_OUT_RECURSION) {
				FindClose(hFind);
				sus_free(filePath);
				sus_free(lpFindFileData);
				return GETTING_OUT_RECURSION;
			}
		}
		else {
			if (!lpFileSearchProc(filePath)) {
				FindClose(hFind);
				sus_free(filePath);
				sus_free(lpFindFileData);
				SUS_PRINTDL("Completion of recursion");
				return GETTING_OUT_RECURSION;
			}
		}
		sus_free(filePath);
	} while (FindNextFileW(hFind, lpFindFileData) != 0);
	sus_free(lpFindFileData);
	FindClose(hFind);
	return 0;
}