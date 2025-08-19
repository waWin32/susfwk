// resapi.h
//
#ifndef _SUS_RES_API_
#define _SUS_RES_API_

// Upload a string resource
LPSTR susLoadStringA(_In_ UINT idString, _Out_opt_ PINT pLen);
// Upload a string resource
LPWSTR susLoadStringW(_In_ UINT idString, _Out_opt_ PINT pLen);

#ifdef UNICODE
#define susLoadString susLoadStringW
#else
#define susLoadString susLoadStringA
#endif // !UNICODE

/*
* Loading a resource of a non-standard type into memory
* @param lpName		Resource name
* @param pdwSize	The optimal value returned is the size of the resource.
* @return TRUE if successful, FALSE otherwise.
*/

// Get a custom resource
LPVOID susLoadResourceA(
	_In_ LPCSTR lpName,
	_In_ LPCSTR lpType,
	_Out_opt_ DWORD* pdwSize,
	_Out_opt_ HGLOBAL* hData
);
// Get a custom resource
LPVOID susLoadResourceW(
	_In_ LPCWSTR lpName,
	_In_ LPCWSTR lpType,
	_Out_opt_ DWORD* pdwSize,
	_Out_opt_ HGLOBAL* hData
);

#ifdef UNICODE
#define susLoadResource susLoadResourceW
#else
#define susLoadResource susLoadResourceA
#endif // !UNICODE

/*
 * Loads a custom resource and saves it to a file.
 * @param lpName    Name of the resource.
 * @param lpFileName Output file path.
 * @return TRUE if successful, FALSE otherwise.
 */

// Upload a resource to a file
BOOL susLoadResourceToFileA(
	_In_ LPCSTR lpName,
	_In_ LPCSTR lpType,
	_In_ LPCSTR lpFileName,
	_In_ DWORD attributes
);
// Upload a resource to a file
BOOL susLoadResourceToFileW(
	_In_ LPCWSTR lpName,
	_In_ LPCWSTR lpType,
	_In_ LPCWSTR lpFileName,
	_In_ DWORD attributes
);

#ifdef UNICODE
#define susLoadResourceToFile susLoadResourceToFileW
#else
#define susLoadResourceToFile susLoadResourceToFileA
#endif // !UNICODE

// Upload a resource to a temporary file
SUS_FILE susLoadResourceToTmpFileA(
	_In_ LPCSTR lpName,
	_In_ LPCSTR lpType,
	_In_ sus_flag32 flags,
	_In_opt_ LPCSTR lpPrefixString,
	_In_opt_ LPCSTR lpFileExtension,
	_Out_opt_ LPSTR lpPath
);
// Upload a resource to a temporary file
SUS_FILE susLoadResourceToTmpFileW(
	_In_ LPCWSTR lpName,
	_In_ LPCWSTR lpType,
	_In_ sus_flag32 flags,
	_In_opt_ LPCWSTR lpPrefixString,
	_In_opt_ LPCWSTR lpFileExtension,
	_Out_opt_ LPWSTR lpPath
);

#ifdef UNICODE
#define susLoadResourceToTmpFile susLoadResourceToTmpFileW
#else
#define susLoadResourceToTmpFile susLoadResourceToTmpFileA
#endif // !UNICODE

#endif /* !_SUS_RES_API_ */