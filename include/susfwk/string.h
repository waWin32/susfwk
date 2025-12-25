// string.h
//
#ifndef _SUS_STRING_
#define _SUS_STRING_

// -----------------------------------------------------------------------------

// Checking whether a character is a space
SUS_INLINE BOOLEAN SUSAPI sus_isspaceA(CHAR c) { return (c == ' ' || c == '\t' || c == '\n' || c == '\r' || c == '\f' || c == '\v'); }
// Checking whether a character is a space
SUS_INLINE BOOLEAN SUSAPI sus_isspaceW(WCHAR c) { return (c == L' ' || c == L'\t' || c == L'\n' || c == L'\r' || c == L'\f' || c == L'\v'); }
// Checking whether a character is a letter
SUS_INLINE BOOLEAN SUSAPI sus_isalphaA(CHAR c) { return(c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z'); }
// Checking whether a character is a letter
SUS_INLINE BOOLEAN SUSAPI sus_isalphaW(WCHAR c) { return (c >= L'A' && c <= L'Z') || (c >= L'a' && c <= L'z'); }
// is a number
SUS_INLINE BOOLEAN SUSAPI sus_isdigitA(CHAR c) { return (c >= '0' && c <= '9'); }
// is a number
SUS_INLINE BOOLEAN SUSAPI sus_isdigitW(WCHAR c) { return (c >= L'0' && c <= L'9'); }
// Is it a number or a word
SUS_INLINE BOOLEAN SUSAPI sus_isalnumA(CHAR c) { return sus_isalphaA(c) || sus_isdigitA(c); }
// Is it a number or a word
SUS_INLINE BOOLEAN SUSAPI sus_isalnumW(WCHAR c) { return sus_isalphaW(c) || sus_isdigitW(c); }

#ifdef UNICODE
#define sus_isspace sus_isspaceW
#define sus_isalpha sus_isalphaW
#define sus_isdigit sus_isdigitW
#define sus_isalnum	sus_isalnumW
#else
#define sus_isspace sus_isspaceA
#define sus_isalpha sus_isalphaA
#define sus_isdigit sus_isdigitA
#define sus_isalnum	sus_isalnumA
#endif // !UNICODE

// Copy Line to Line
#define sus_strcpy(str1, str2) lstrcpyA(str1, str2)
// Copy Line to Line
#define sus_wcscpy(str1, str2) lstrcpyW(str1, str2)

// Get length of the string
#define sus_strlen(str) lstrlenA(str)
// Get length of the string
#define sus_wcslen(str) lstrlenW(str)

// Detailed string comparison
#define sus_strcmp(str1, str2) ((BOOL)(!lstrcmpA(str1, str2) ? TRUE : FALSE))
// Detailed string comparison
#define sus_wcscmp(str1, str2) ((BOOL)(!lstrcmpW(str1, str2) ? TRUE : FALSE))

// Comparing strings without case checking
#define sus_strcmpi(str1, str2) ((BOOL)(!lstrcmpiA(str1, str2) ? TRUE : FALSE))
// Comparing strings without case checking
#define sus_wcscmpi(str1, str2) ((BOOL)(!lstrcmpiW(str1, str2) ? TRUE : FALSE))

// -----------------------------------------------------------------------------

// Convert ascii format to unicode
VOID SUSAPI sus_atow(_In_ LPCSTR source, _Out_ LPWSTR buffer);
// Convert Unicode format to Ascii
VOID SUSAPI sus_wtoa(_In_ LPCWSTR source, _Out_ LPSTR buffer);

// Flipping the string
VOID SUSAPI sus_strrev(_Inout_ LPSTR str);
// Flipping the string
VOID SUSAPI sus_wcsrev(_Inout_ LPWSTR str);

#ifdef UNICODE
#define sus_tcsrev	sus_wcsrev
#else
#define sus_tcsrev	sus_strrev
#endif // !UNICODE

// -----------------------------------------------------------------------------

// convert string to int
LONGLONG SUSAPI sus_atoi(_In_ LPCSTR str, _Out_opt_ LPCSTR* end);
// convert string to int
LONGLONG SUSAPI sus_wtoi(_In_ LPCWSTR str, _Out_opt_ LPCWSTR* end);

#ifdef UNICODE
#define sus_stoi	sus_wtoi
#else
#define sus_stoi	sus_atoi
#endif // !UNICODE

// convert string to float
FLOAT SUSAPI sus_atof(_In_ LPCSTR str, _Out_opt_ LPCSTR* end);
// convert string to float
FLOAT SUSAPI sus_wtof(_In_ LPCWSTR str, _Out_opt_ LPCWSTR* end);

#ifdef UNICODE
#define sus_stof	sus_wtof
#else
#define sus_stof	sus_atof
#endif // !UNICODE

// -----------------------------------------------------------------------------

// convert int to string
LPSTR SUSAPI sus_itoa(
	_Out_ LPSTR buffer,
	_In_ LONGLONG value
);
// convert int to string
LPWSTR SUSAPI sus_itow(
	_Out_ LPWSTR buffer,
	_In_ LONGLONG value
);
// Number length
SUS_INLINE INT SUSAPI sus_islen(_In_ INT64 value) {
	CHAR buff[12];
	return (int)(sus_itoa(buff, value) - buff) / sizeof(CHAR);
}

#ifdef UNICODE
#define sus_itos	sus_itow
#else
#define sus_itos	sus_itoa
#endif // !UNICODE

// convert float to string
LPSTR SUSAPI sus_ftoa(
	_Out_ LPSTR buffer,
	_In_ FLOAT value,
	_In_ DWORD precision
);
// convert float to string
LPWSTR SUSAPI sus_ftow(
	_Out_ LPWSTR buffer,
	_In_ FLOAT value,
	_In_ DWORD precision
);
// Number length
SUS_INLINE INT SUSAPI sus_fslen(_In_ FLOAT value, _In_ UINT precision) {
	CHAR buff[18];
	return (int)(sus_ftoa(buff, value, precision) - buff) / sizeof(CHAR);
}

#ifdef UNICODE
#define sus_ftos	sus_ftow
#else
#define sus_ftos	sus_ftoa
#endif // !UNICODE

// -----------------------------------------------------------------------------

// cropping a string
VOID SUSAPI sus_substring(
	_Out_ LPSTR buffer,
	_In_ LPCSTR str,
	_In_ DWORD substart,
	_In_ DWORD subend
);
// cropping a string
VOID SUSAPI sus_subwstring(
	_Out_ LPWSTR buffer,
	_In_ LPCWSTR str,
	_In_ DWORD substart,
	_In_ DWORD subend
);

#ifdef UNICODE
#define sus_subtstring	sus_subwstring
#else
#define sus_subtstring	sus_substring
#endif // !UNICODE

// Separation from spaces
LPSTR SUSAPI sus_trimlA(
	_Inout_ LPSTR* str
);
// Separation from spaces
LPWSTR SUSAPI sus_trimlW(
	_Inout_ LPWSTR* str
);

#ifdef UNICODE
#define sus_triml	sus_trimlW
#else
#define sus_triml	sus_trimlA
#endif // !UNICODE

// Separation from spaces
LPSTR SUSAPI sus_trimrA(
	_Inout_ LPSTR str
);
// Separation from spaces
LPWSTR SUSAPI sus_trimrW(
	_Inout_ LPWSTR str
);

#ifdef UNICODE
#define sus_trimr	sus_trimrW
#else
#define sus_trimr	sus_trimrA
#endif // !UNICODE

// Separation from spaces
LPSTR SUSAPI sus_trimA(
	_Inout_ LPSTR* str
);
// Separation from spaces
LPWSTR SUSAPI sus_trimW(
	_Inout_ LPWSTR* str
);

#ifdef UNICODE
#define sus_trim	sus_trimW
#else
#define sus_trim	sus_trimA
#endif // !UNICODE

// -----------------------------------------------------------------------------

// The index of the symbol
LPSTR SUSAPI sus_strchr(
	_In_ LPCSTR str,
	_In_ CHAR s
);
// The index of the symbol
LPWSTR SUSAPI sus_wcschr(
	_In_ LPCWSTR str,
	_In_ WCHAR s
);

// Find a rock undergrowth in a row
LPSTR SUSAPI sus_strstr(
	_In_ LPCSTR str,
	_In_ LPCSTR substring
);
// Find a rock undergrowth in a row
LPWSTR SUSAPI sus_wcsstr(
	_In_ LPCWSTR str,
	_In_ LPCWSTR substring
);

// The last index of the symbol
LPSTR SUSAPI sus_strrchr(
	_In_ LPCSTR str,
	_In_ CHAR s
);
// The last index of the symbol
LPWSTR SUSAPI sus_wcsrchr(
	_In_ LPCWSTR str,
	_In_ WCHAR s
);

// Find a rock undergrowth in a row
LPSTR SUSAPI sus_strrstr(
	_In_ LPCSTR str,
	_In_ LPCSTR substring
);
// Find a rock undergrowth in a row
LPWSTR SUSAPI sus_wcsrstr(
	_In_ LPCWSTR str,
	_In_ LPCWSTR substring
);

// parse a string for tokens
LPSTR SUSAPI sus_strtok(
	_Inout_ LPSTR* ctx,
	_In_ LPCSTR delimiter
);
// parse a string for tokens
LPWSTR SUSAPI sus_wcstok(
	_Inout_ LPWSTR* ctx,
	_In_ LPCWSTR delimiter
);

// -----------------------------------------------------------------------------

#endif /* !_SUS_STRING_ */
