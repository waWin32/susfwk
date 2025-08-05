// regapi.h
//
#ifndef _SUS_REG_API_
#define _SUS_REG_API_

// Open the key in the registry
HKEY SUSAPI susRegOpenKeyA(
	_In_ HKEY hKey,
	_In_opt_ LPCSTR subKey
);
// Open the key in the registry
HKEY SUSAPI susRegOpenKeyW(
	_In_ HKEY hKey,
	_In_opt_ LPCWSTR subKey
);

#ifdef UNICODE
#define susRegOpenKey susRegOpenKeyW
#else
#define susRegOpenKey susRegOpenKeyA
#endif // !UNICODE

// Create the key in the registry
HKEY SUSAPI susRegCreateKeyA(
	_In_ HKEY hKey,
	_In_ LPCSTR subKey
);
// Create the key in the registry
HKEY SUSAPI susRegCreateKeyW(
	_In_ HKEY hKey,
	_In_ LPCWSTR subKey
);

#ifdef UNICODE
#define susRegCreateKey susRegCreateKeyW
#else
#define susRegCreateKey susRegCreateKeyA
#endif // !UNICODE

// Writing a value to a key
BOOL SUSAPI susRegSetValueA(
	_In_ HKEY hKey,
	_In_ LPCSTR valueName,
	_In_ DWORD type,
	_In_ LPBYTE lpData,
	_In_ DWORD cbData
);
// Writing a value to a key
BOOL SUSAPI susRegSetValueW(
	_In_ HKEY hKey,
	_In_ LPCWSTR valueName,
	_In_ DWORD type,
	_In_ LPBYTE lpData,
	_In_ DWORD cbData
);

#ifdef UNICODE
#define susRegSetValue susRegSetValueW
#else
#define susRegSetValue susRegSetValueA
#endif // !UNICODE

// Getting a value to a key
BOOL SUSAPI susRegGetValueA(
	_In_ HKEY hKey,
	_In_opt_ LPSTR lpSubKey,
	_In_ LPCSTR valueName,
	_In_ DWORD flags,
	_Out_ LPVOID lpData,
	_Inout_ PDWORD cbData
);
// Getting a value to a key
BOOL SUSAPI susRegGetValueW(
	_In_ HKEY hKey,
	_In_opt_ LPWSTR lpSubKey,
	_In_ LPCWSTR valueName,
	_In_ DWORD flags,
	_Out_ LPVOID lpData,
	_Inout_ PDWORD cbData
);

#ifdef UNICODE
#define susRegGetValue susRegGetValueW
#else
#define susRegGetValue susRegGetValueA
#endif // !UNICODE

// Delete a key
BOOL SUSAPI susRegDeleteKeyA(
	_In_ HKEY hKey,
	_In_ LPCSTR lpSubKey
);
// Delete a key
BOOL SUSAPI susRegDeleteKeyW(
	_In_ HKEY hKey,
	_In_ LPCWSTR lpSubKey
);

#ifdef UNICODE
#define susRegDeleteKey susRegDeleteKeyW
#else
#define susRegDeleteKey susRegDeleteKeyA
#endif // !UNICODE

#endif /* !_SUS_REG_API_ */