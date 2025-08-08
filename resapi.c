// resapi.c
//
#include "coreframe.h"
#include "include/susfwk/core.h"
#include "include/susfwk/iostream.h"
#include "include/susfwk/memory.h"
#include "include/susfwk/resapi.h"

/*
* Loading a resource of a non-standard type into memory
* @param lpName		Resource name
* @param pdwSize	The optimal value returned is the size of the resource.
* @return TRUE if successful, FALSE otherwise.
*/

// Upload a string resource
LPSTR susLoadStringA(_In_ UINT idString, _Out_opt_ PINT pLen)
{
    SUS_PRINTDL("Loading a string resource");
    HMODULE hModule = GetModuleHandleA(NULL);
    static CHAR dummy;
    int len = LoadStringA(
        hModule,
        idString,
        &dummy,
        0xfffffff
    );
    if (pLen) *pLen = len;
    if (len <= 0) {
        SUS_PRINTDE("Failed to load string resource");
        SUS_PRINTDC(GetLastError());
        return NULL;
    }
    LPSTR buff = sus_malloc((SIZE_T)len + 1);
    if (!buff) {
        SUS_PRINTDE("Failed to load string resource");
        return NULL;
    }
    if (LoadStringA(
        hModule,
        idString,
        buff,
        len + 1
    ) <= 0) {
        SUS_PRINTDE("Failed to load string resource");
        SUS_PRINTDC(GetLastError());
        sus_free(buff);
        return NULL;
    }
    SUS_PRINTDL("A %d-byte resource has been successfully uploaded!", len);
    return buff;
}
// Upload a string resource
LPWSTR susLoadStringW(_In_ UINT idString, _Out_opt_ PINT pLen)
{
    SUS_PRINTDL("Loading a string resource");
    HMODULE hModule = GetModuleHandleW(NULL);
    static WCHAR dummy;
    int len = LoadStringW(
        hModule,
        idString,
        &dummy,
        0xfffffff
    );
    if (pLen) *pLen = len;
    if (len <= 0) {
        SUS_PRINTDE("Failed to load string resource");
        SUS_PRINTDC(GetLastError());
        return NULL;
    }
    LPWSTR buff = sus_malloc((SIZE_T)len + 1);
    if (!buff) {
        SUS_PRINTDE("Failed to load string resource");
        return NULL;
    }
    if (LoadStringW(
        hModule,
        idString,
        buff,
        len + 1
    ) <= 0) {
        SUS_PRINTDE("Failed to load string resource");
        SUS_PRINTDC(GetLastError());
        sus_free(buff);
        return NULL;
    }
    SUS_PRINTDL("A %d-byte resource has been successfully uploaded!", len);
    return buff;
}
// Get a custom resource
LPVOID susLoadResourceA(
	_In_ LPCSTR lpName,
    _In_ LPCSTR lpType,
	_Out_opt_ DWORD *pdwSize,
    _Out_opt_ HGLOBAL *hData)
{
    SUS_PRINTDL("Getting a custom resource");
    if (hData) *hData = NULL;
    SUS_ASSERT(lpName && lpType);
    HRSRC hRes = FindResourceA(NULL, lpName, lpType);
    if (!hRes) {
        SUS_PRINTDE("Resource not found!");
        if (pdwSize) *pdwSize = 0;
        return NULL;
    }
    if (pdwSize) {
        *pdwSize = SizeofResource(NULL, hRes);
        if (*pdwSize == 0) {
            SUS_PRINTDW("Resource size is zero!");
            SUS_PRINTDC(GetLastError());
        }
    }
    HGLOBAL _hData = LoadResource(NULL, hRes);
    if (!_hData) {
        SUS_PRINTDE("Could not load resource!");
        SUS_PRINTDC(GetLastError());
        return NULL;
    }
    LPVOID pData = LockResource(_hData);
    if (!pData) {
        SUS_PRINTDE("Could not lock resource '%s'", lpName);
        SUS_PRINTDC(GetLastError());
        return NULL;
    }
    if (hData) *hData = _hData;
    SUS_PRINTDL("Custom resources received");
    return pData;
}
// Get a custom resource
LPVOID susLoadResourceW(
    _In_ LPCWSTR lpName,
    _In_ LPCWSTR lpType,
    _Out_opt_ DWORD* pdwSize,
    _Out_opt_ HGLOBAL* hData)
{
    SUS_PRINTDL("Getting a custom resource");
    if (hData) *hData = NULL;
    SUS_ASSERT(lpName && lpType);
    HRSRC hRes = FindResourceW(NULL, lpName, lpType);
    if (!hRes) {
        SUS_PRINTDE("Resource not found!");
        if (pdwSize) *pdwSize = 0;
        return NULL;
    }
    if (pdwSize) {
        *pdwSize = SizeofResource(NULL, hRes);
        if (*pdwSize == 0) {
            SUS_PRINTDW("Resource size is zero!");
            SUS_PRINTDC(GetLastError());
        }
    }
    HGLOBAL _hData = LoadResource(NULL, hRes);
    if (!_hData) {
        SUS_PRINTDE("Could not load resource!");
        SUS_PRINTDC(GetLastError());
        return NULL;
    }
    LPVOID pData = LockResource(_hData);
    if (!pData) {
        SUS_PRINTDE("Could not lock resource '%s'", lpName);
        SUS_PRINTDC(GetLastError());
        return NULL;
    }
    if (hData) *hData = _hData;
    SUS_PRINTDL("Custom resources received");
    return pData;
}

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
    _In_ DWORD attributes)
{
    SUS_PRINTDL("Uploading a resource to a file");
    DWORD dwSize;
    LPVOID pData = susLoadResourceA(lpName, lpType , &dwSize, NULL);
    if (pData == NULL) {
        SUS_PRINTDE("Failed to load resource");
        return FALSE;
    }
    SUS_FILE hFres = sus_fcreateA(lpFileName, attributes);
    if (!hFres) {
        SUS_PRINTDE("Error: Could not create output file!");
        return FALSE;
    }
    if (!sus_fwrite(hFres, pData, dwSize)) {
        susDeleteFileA(lpFileName);
        sus_fclose(hFres);
        SUS_PRINTDE("Error: Could not write to the file!");
        return FALSE;
    }
    sus_fclose(hFres);
    SUS_PRINTDL("The resource has been successfully saved to a file");
    return TRUE;
}

// Upload a resource to a file
BOOL susLoadResourceToFileW(
    _In_ LPCWSTR lpName,
    _In_ LPCWSTR lpType,
    _In_ LPCWSTR lpFileName,
    _In_ DWORD attributes)
{
    SUS_PRINTDL("Uploading a resource to a file");
    DWORD dwSize;
    LPVOID pData = susLoadResourceW(lpName, lpType, &dwSize, NULL);
    if (pData == NULL) {
        SUS_PRINTDE("Failed to load resource");
        return FALSE;
    }
    SUS_FILE hFres = sus_fcreateW(lpFileName, attributes);
    if (!hFres) {
        SUS_PRINTDE("Error: Could not create output file!");
        return FALSE;
    }
    if (!sus_fwrite(hFres, pData, dwSize)) {
        susDeleteFileW(lpFileName);
        sus_fclose(hFres);
        SUS_PRINTDE("Error: Could not write to the file!");
        return FALSE;
    }
    sus_fclose(hFres);
    SUS_PRINTDL("The resource has been successfully saved to a file");
    return TRUE;
}

// Upload a resource to a temporary file
SUS_FILE susLoadResourceToTmpFileA(
    _In_ LPCSTR lpName,
    _In_ LPCSTR lpType,
    _In_ SUS_FLAG16 flags,
    _In_opt_ LPCSTR lpPrefixString,
    _In_opt_ LPCSTR lpFileExtension,
    _Out_opt_ LPSTR lpPath)
{
    SUS_PRINTDL("Uploading a resource to a temporary file");
    DWORD dwSize;
    if (lpPath) *lpPath = '\0';
    LPVOID pData = susLoadResourceA(lpName, lpType, &dwSize, NULL);
    if (pData == NULL) {
        SUS_PRINTDE("Failed to load resource");
        return NULL;
    }
    CHAR lpTempFileName[MAX_PATH];
    SUS_FILE hFile = susCreateTempFileA(lpPrefixString, lpFileExtension, flags, lpTempFileName);
    if (!hFile) {
        SUS_PRINTDE("Failed to load resource");
        return NULL;
    }
    if (!sus_fwrite(hFile, pData, dwSize)) {
        sus_fclose(hFile);
        if (!(flags & SUS_TEMP_FILE_DELETE_ON_CLOSE)) susDeleteFileA(lpTempFileName);
        SUS_PRINTDE("Failed to load resource");
        return NULL;
    }
    if (lpPath) lstrcpyA(lpPath, lpTempFileName);
    SUS_PRINTDL("The resource has been successfully uploaded to a file");
    return hFile;
}
// Upload a resource to a temporary file
SUS_FILE susLoadResourceToTmpFileW(
    _In_ LPCWSTR lpName,
    _In_ LPCWSTR lpType,
    _In_ SUS_FLAG16 flags,
    _In_opt_ LPCWSTR lpPrefixString,
    _In_opt_ LPCWSTR lpFileExtension,
    _Out_opt_ LPWSTR lpPath)
{
    SUS_PRINTDL("Uploading a resource to a temporary file");
    DWORD dwSize;
    if (lpPath) *lpPath = L'\0';
    LPVOID pData = susLoadResourceW(lpName, lpType, &dwSize, NULL);
    if (pData == NULL) {
        SUS_PRINTDE("Failed to load resource");
        return NULL;
    }
    WCHAR lpTempFileName[MAX_PATH];
    SUS_FILE hFile = susCreateTempFileW(lpPrefixString, lpFileExtension, flags, lpTempFileName);
    if (!hFile) {
        SUS_PRINTDE("Failed to load resource");
        return NULL;
    }
    if (!sus_fwrite(hFile, pData, dwSize)) {
        sus_fclose(hFile);
        if (!(flags & SUS_TEMP_FILE_DELETE_ON_CLOSE)) susDeleteFileW(lpTempFileName);
        SUS_PRINTDE("Failed to load resource");
        return NULL;
    }
    if (lpPath) lstrcpyW(lpPath, lpTempFileName);
    SUS_PRINTDL("The resource has been successfully uploaded to a file");
    return hFile;
}