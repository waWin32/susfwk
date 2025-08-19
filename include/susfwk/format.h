// format.h
//
#ifndef _SUS_FORMAT_
#define _SUS_FORMAT_

// text formatting
INT SUSAPI sus_vformattingA(
	_Out_opt_ LPSTR buffer,
	_In_ _Printf_format_string_ LPCSTR format,
	_In_ sus_va_list args
);
// text formatting
INT SUSAPI sus_vformattingW(
	_Out_opt_ LPWSTR buffer,
	_In_ _Printf_format_string_ LPCWSTR format,
	_In_ sus_va_list args
);

#ifdef UNICODE
#define sus_vformatting	sus_vformattingW
#else
#define sus_vformatting	sus_vformattingA
#endif // !UNICODE

// Get a dynamically formatted string
SUS_INLINE LPSTR SUSAPI sus_dvformattingA(	_In_ _Printf_format_string_ LPCSTR format,
											_In_ sus_va_list args) {
	INT len = sus_vformattingA(NULL, format, args);
	LPSTR lpFormattedString = HeapAlloc(GetProcessHeap(), 0, ((sus_size_t)len + 1) * sizeof(CHAR));
	if (!lpFormattedString) return NULL;
	sus_vformattingA(lpFormattedString, format, args);
	return lpFormattedString;
}
// Get a dynamically formatted string
SUS_INLINE LPWSTR SUSAPI sus_dvformattingW(	_In_ _Printf_format_string_ LPCWSTR format,
											_In_ sus_va_list args) {
	INT len = sus_vformattingW(NULL, format, args);
	LPWSTR lpFormattedString = HeapAlloc(GetProcessHeap(), 0, ((sus_size_t)len + 1) * sizeof(CHAR));
	if (!lpFormattedString) return NULL;
	sus_vformattingW(lpFormattedString, format, args);
	return lpFormattedString;
}

#ifdef UNICODE
#define sus_dvformatting	sus_dvformattingW
#else
#define sus_dvformatting	sus_dvformattingA
#endif // !UNICODE

// text formatting
INT SUSAPIV sus_formattingA(
	_Out_opt_ LPSTR buffer,
	_In_ _Printf_format_string_ LPCSTR format,
	_In_ ...
);
// text formatting
INT SUSAPIV sus_formattingW(
	_Out_opt_ LPWSTR buffer,
	_In_ _Printf_format_string_ LPCWSTR format,
	_In_ ...
);

#ifdef UNICODE
#define sus_formatting	sus_formattingW
#else
#define sus_formatting	sus_formattingA
#endif // !UNICODE

// Get a dynamically formatted string
SUS_INLINE LPSTR SUSAPI sus_dformattingA(	_In_ _Printf_format_string_ LPCSTR format,
											_In_ ...) {
	sus_va_list args;
	sus_va_start(args, format);
	INT len = sus_vformattingA(NULL, format, args);
	LPSTR lpFormattedString = HeapAlloc(GetProcessHeap(), 0, ((sus_size_t)len + 1) * sizeof(CHAR));
	if (!lpFormattedString) return NULL;
	sus_vformattingA(lpFormattedString, format, args);
	sus_va_end(args);
	return lpFormattedString;
}
// Get a dynamically formatted string
SUS_INLINE LPWSTR SUSAPI sus_dformattingW(	_In_ _Printf_format_string_ LPCWSTR format,
											_In_ ...) {
	sus_va_list args;
	sus_va_start(args, format);
	INT len = sus_vformattingW(NULL, format, args);
	LPWSTR lpFormattedString = HeapAlloc(GetProcessHeap(), 0, ((sus_size_t)len + 1) * sizeof(CHAR));
	if (!lpFormattedString) return NULL;
	sus_vformattingW(lpFormattedString, format, args);
	sus_va_end(args);
	return lpFormattedString;
}

#ifdef UNICODE
#define sus_dformatting	sus_dformattingW
#else
#define sus_dformatting	sus_dformattingA
#endif // !UNICODE

// Parsing text into variables
INT sus_vparsingA(
	_In_ LPCSTR str,
	_In_ _Printf_format_string_ LPCSTR format,
	_In_ sus_va_list args
);
// Parsing text into variables
INT sus_vparsingW(
	_In_ LPCWSTR str,
	_In_ _Printf_format_string_ LPCWSTR format,
	_In_ sus_va_list args
);

#ifdef UNICODE
#define sus_vparsing	sus_vparsingW
#else
#define sus_vparsing	sus_vparsingA
#endif // !UNICODE


// Parsing text into variables
INT sus_parsingA(
	_In_ LPCSTR str,
	_In_ _Printf_format_string_ LPCSTR format,
	_In_ ...
);
// Parsing text into variables
INT sus_parsingW(
	_In_ LPCWSTR str,
	_In_ _Printf_format_string_ LPCWSTR format,
	_In_ ...
);

#ifdef UNICODE
#define sus_parsing	sus_parsingW
#else
#define sus_parsing	sus_parsingA
#endif // !UNICODE

#endif /* !_SUS_FORMAT_ */