// susstring.h
//
#ifndef _SUS_STRING_
#define _SUS_STRING_

typedef struct sus_string_buffer {
	LPSTR string;
	DWORD len;
} SUS_STRING_BUFFER;

typedef struct sus_wstring_buffer {
	LPSTR string;
	DWORD len;
} SUS_WSTRING_BUFFER;

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

// Get the length of the string
SUS_INLINE INT SUSAPI sus_strlenA(LPCSTR lpString) {
	return lstrlenA(lpString);
}
// Get the length of the string
SUS_INLINE INT SUSAPI sus_strlenW(LPCWSTR lpString) {
	return lstrlenW(lpString);
}

#ifdef UNICODE
#define sus_strlen	sus_strlenW
#else
#define sus_strlen	sus_strlenA
#endif // !UNICODE

// Flipping the string
VOID SUSAPI sus_strrevA(_Inout_ LPSTR str);
// Flipping the string
VOID SUSAPI sus_strrevW(_Inout_ LPWSTR str);

#ifdef UNICODE
#define sus_strrev	sus_strrevW
#else
#define sus_strrev	sus_strrevA
#endif // !UNICODE

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

// cropping a string
VOID SUSAPI sus_substringA(
	_Out_ LPSTR buffer,
	_In_ LPCSTR str,
	_In_ DWORD substart,
	_In_ DWORD subend
);
// cropping a string
VOID SUSAPI sus_substringW(
	_Out_ LPWSTR buffer,
	_In_ LPCWSTR str,
	_In_ DWORD substart,
	_In_ DWORD subend
);

#ifdef UNICODE
#define sus_substring	sus_substringW
#else
#define sus_substring	sus_substringA
#endif // !UNICODE

// The index of the symbol
LPSTR SUSAPI sus_strchrA(
	_In_ LPCSTR str,
	_In_ CHAR s
);
// The index of the symbol
LPWSTR SUSAPI sus_strchrW(
	_In_ LPCWSTR str,
	_In_ WCHAR s
);

#ifdef UNICODE
#define sus_strchr	sus_strchrW
#else
#define sus_strchr	sus_strchrA
#endif // !UNICODE

// The last index of the symbol
LPSTR SUSAPI sus_strrchrA(
	_In_ LPCSTR str,
	_In_ CHAR s
);
// The last index of the symbol
LPWSTR SUSAPI sus_strrchrW(
	_In_ LPCWSTR str,
	_In_ WCHAR s
);

#ifdef UNICODE
#define sus_strrchr	sus_strrchrW
#else
#define sus_strrchr	sus_strrchrA
#endif // !UNICODE

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

#endif /* !_SUS_STRING_ */
