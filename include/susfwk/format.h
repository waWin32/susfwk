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
#define _sus_formatting	_sus_formattingW
#else
#define _sus_formatting	_sus_formattingA
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