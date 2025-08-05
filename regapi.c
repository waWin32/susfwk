// regapi.c
//
#include "coreframe.h"
#include "include/susfwk/core.h"
#include "include/susfwk/memory.h"
#include "include/susfwk/regapi.h"

// Open the key in the registry
HKEY SUSAPI susRegOpenKeyA(
	_In_ HKEY hKey,
	_In_opt_ LPCSTR subKey)
{
	SUS_PRINTDL("Opening a key in the registry");
	HKEY hCurKey;
	if (RegOpenKeyExA(hKey, subKey, 0, KEY_ALL_ACCESS, &hCurKey) != ERROR_SUCCESS)
	{
		SUS_PRINTDE("Couldn't open the key");
		SUS_PRINTDC(GetLastError());
		return NULL;
	}
	SUS_PRINTDL("The key has been opened successfully");
	return hCurKey;
}
// Open the key in the registry
HKEY SUSAPI susRegOpenKeyW(
	_In_ HKEY hKey,
	_In_opt_ LPCWSTR subKey)
{
	SUS_PRINTDL("Opening a key in the registry");
	HKEY hCurKey;
	if (RegOpenKeyExW(hKey, subKey, 0, KEY_ALL_ACCESS, &hCurKey) != ERROR_SUCCESS)
	{
		SUS_PRINTDE("Couldn't open the key");
		SUS_PRINTDC(GetLastError());
		return NULL;
	}
	SUS_PRINTDL("The key has been opened successfully");
	return hCurKey;
}

// Create the key in the registry
HKEY SUSAPI susRegCreateKeyA(
	_In_ HKEY hKey,
	_In_ LPCSTR subKey)
{
	SUS_PRINTDL("Creating a key in the registry");
	HKEY hCurKey;
	if (RegCreateKeyExA(
		hKey, subKey, 0, NULL, 0,
		KEY_ALL_ACCESS, NULL, &hCurKey, NULL
	) != ERROR_SUCCESS)
	{
		SUS_PRINTDE("Couldn't Create the key");
		SUS_PRINTDC(GetLastError());
		return NULL;
	}
	SUS_PRINTDL("The key has been Created successfully");
	return hCurKey;
}
// Create the key in the registry
HKEY SUSAPI susRegCreateKeyW(
	_In_ HKEY hKey,
	_In_ LPCWSTR subKey)
{
	SUS_PRINTDL("Creating a key in the registry");
	HKEY hCurKey;
	if (RegCreateKeyExW(
		hKey, subKey, 0, NULL, 0,
		KEY_ALL_ACCESS, NULL, &hCurKey, NULL
	) != ERROR_SUCCESS)
	{
		SUS_PRINTDE("Couldn't Create the key");
		SUS_PRINTDC(GetLastError());
		return NULL;
	}
	SUS_PRINTDL("The key has been Created successfully");
	return hCurKey;
}
// Writing a value to a key
BOOL SUSAPI susRegSetValueA(
	_In_ HKEY hKey,
	_In_ LPCSTR valueName,
	_In_ DWORD type,
	_In_ LPBYTE lpData,
	_In_ DWORD cbData)
{
	SUS_PRINTDL("Writing a value to a key");
	if (RegSetValueExA(hKey, valueName, 0, type, lpData, cbData) != ERROR_SUCCESS)
	{
		SUS_PRINTDE("Couldn't write data to the key");
		SUS_PRINTDC(GetLastError());
		return FALSE;
	}
	SUS_PRINTDL("Data has been successfully recorded");
	return TRUE;
}
// Writing a value to a key
BOOL SUSAPI susRegSetValueW(
	_In_ HKEY hKey,
	_In_ LPCWSTR valueName,
	_In_ DWORD type,
	_In_ LPBYTE lpData,
	_In_ DWORD cbData)
{
	SUS_PRINTDL("Writing a value to a key");
	if (RegSetValueExW(hKey, valueName, 0, type, lpData, cbData) != ERROR_SUCCESS)
	{
		SUS_PRINTDE("Couldn't write data to the key");
		SUS_PRINTDC(GetLastError());
		return FALSE;
	}
	SUS_PRINTDL("Data has been successfully recorded");
	return TRUE;
}
// Getting a value to a key
BOOL SUSAPI susRegGetValueA(
	_In_ HKEY hKey,
	_In_opt_ LPSTR lpSubKey,
	_In_ LPCSTR valueName,
	_In_ DWORD flags,
	_Out_ LPVOID lpData,
	_Inout_ PDWORD cbData)
{
	SUS_PRINTDL("Getting a value to a key");
	if (RegGetValueA(hKey, lpSubKey, valueName, flags, NULL, lpData, cbData) != ERROR_SUCCESS)
	{
		SUS_PRINTDE("Couldn't get the data in the key");
		SUS_PRINTDC(GetLastError());
		return FALSE;
	}
	SUS_PRINTDL("Data has been successfully getted");
	return TRUE;
}
// Getting a value to a key
BOOL SUSAPI susRegGetValueW(
	_In_ HKEY hKey,
	_In_opt_ LPWSTR lpSubKey,
	_In_ LPCWSTR valueName,
	_In_ DWORD flags,
	_Out_ LPVOID lpData,
	_Inout_ PDWORD cbData)
{
	SUS_PRINTDL("Getting a value to a key");
	if (RegGetValueW(hKey, lpSubKey, valueName, flags, NULL, lpData, cbData) != ERROR_SUCCESS)
	{
		SUS_PRINTDE("Couldn't get the data in the key");
		SUS_PRINTDC(GetLastError());
		return FALSE;
	}
	SUS_PRINTDL("Data has been successfully getted");
	return TRUE;
}
// Delete a key
BOOL SUSAPI susRegDeleteKeyA(
	_In_ HKEY hKey,
	_In_ LPCSTR lpSubKey)
{
	SUS_PRINTDL("Deleting a key");
	if (RegDeleteKeyA(hKey, lpSubKey) != ERROR_SUCCESS)
	{
		SUS_PRINTDE("Couldn't delete the key");
		SUS_PRINTDC(GetLastError());
		return FALSE;
	}
	SUS_PRINTDL("The key was successfully deleted");
	return TRUE;
}
// Delete a key
BOOL SUSAPI susRegDeleteKeyW(
	_In_ HKEY hKey,
	_In_ LPCWSTR lpSubKey)
{
	SUS_PRINTDL("Deleting a key");
	if (RegDeleteKeyW(hKey, lpSubKey) != ERROR_SUCCESS)
	{
		SUS_PRINTDE("Couldn't delete the key");
		SUS_PRINTDC(GetLastError());
		return FALSE;
	}
	SUS_PRINTDL("The key was successfully deleted");
	return TRUE;
}