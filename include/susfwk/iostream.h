// SUS iostream.h
//
#ifndef _SUS_IO_STREAM_
#define _SUS_IO_STREAM_

#include "fileio.h"
#include "conio.h"

// --------------------------------------------------------

// Write data to a file
SUS_INLINE INT SUSAPI sus_writeA(_In_ SUS_FILE hOut, LPCSTR text, INT len)
{
	if (len < 0) len = lstrlenA(text);
	DWORD mode;
	if (GetConsoleMode(hOut, &mode))
		return sus_cwriteA(hOut, text, len);
	return sus_fwrite(hOut, (CONST LPBYTE)text, len * sizeof(CHAR));
}
// Write data to a file
SUS_INLINE INT SUSAPI sus_writeW(_In_ SUS_FILE hOut, LPCWSTR text, INT len)
{
	if (len < 0) len = lstrlenW(text);
	DWORD mode;
	if (GetConsoleMode(hOut, &mode))
		return sus_cwriteW(hOut, text, len);
	return sus_fwrite(hOut, (CONST LPBYTE)text, len * sizeof(WCHAR));
}

#ifdef UNICODE
#define sus_write	sus_writeW
#else
#define sus_write	sus_writeA
#endif // !UNICODE

// Read data in a file
SUS_INLINE INT SUSAPI sus_readA(_In_ SUS_FILE hIn, LPSTR buff, INT len)
{
	DWORD mode;
	if (GetConsoleMode(hIn, &mode))
		return sus_creadA(hIn, buff, len);
	return sus_freadex(hIn, (LPBYTE)buff, (DWORD)len * (DWORD)sizeof(CHAR));
}
// Read data in a file
SUS_INLINE INT SUSAPI sus_readW(_In_ SUS_FILE hIn, LPWSTR buff, INT len)
{
	DWORD mode;
	if (GetConsoleMode(hIn, &mode))
		return sus_creadW(hIn, buff, len);
	return sus_freadex(hIn, (LPBYTE)buff, (DWORD)len * (DWORD)sizeof(WCHAR));
}

#ifdef UNICODE
#define sus_read	sus_readW
#else
#define sus_read	sus_readA
#endif // !UNICODE

// --------------------------------------------------------

// Formatted output to the file
INT SUSAPI sus_vfprintfA(
	_In_ SUS_FILE hFile,
	_In_ LPCSTR format,
	_In_ sus_va_list args
);
// Formatted output to the file
SUS_INLINE INT SUSAPIV sus_fprintfA(	
	_In_ SUS_FILE hOut,
	_In_ LPCSTR format,
	_In_ ...) 
{
	sus_va_list args;
	sus_va_start(args, format);
	int res = sus_vfprintfA(hOut, format, args);
	sus_va_end(args);
	return res;
}
// Formatted output to the file
INT SUSAPI sus_vfprintfW(
	_In_ SUS_FILE hFile,
	_In_ LPCWSTR format,
	_In_ sus_va_list args
);
// Formatted output to the file
SUS_INLINE INT SUSAPIV sus_fprintfW(
	_In_ SUS_FILE hOut,
	_In_ LPCWSTR format,
	_In_ ...)
{
	sus_va_list args;
	sus_va_start(args, format);
	int res = sus_vfprintfW(hOut, format, args);
	sus_va_end(args);
	return res;
}

#ifdef UNICODE
#define sus_vfprintf	sus_vfprintfW
#define sus_fprintf		sus_fprintfW
#else
#define sus_vfprintf	sus_vfprintfA
#define sus_fprintf		sus_fprintfA
#endif // !UNICODE

// --------------------------------------------------------

// Formatted input to the file
INT SUSAPI sus_vfscanfA(
	_In_ SUS_FILE hIn,
	_In_ LPCSTR format,
	_Inout_ sus_va_list args
);
// Formatted output to the file
SUS_INLINE INT SUSAPIV sus_fscanfA(
	_In_ SUS_FILE hIn,
	_In_ LPCSTR format,
	_Inout_ ...)
{
	sus_va_list args;
	sus_va_start(args, format);
	int res = sus_vfscanfA(hIn, format, args);
	sus_va_end(args);
	return res;
}
// Formatted input to the file
INT SUSAPI sus_vfscanfW(
	_In_ SUS_FILE hIn,
	_In_ LPCWSTR format,
	_Inout_ sus_va_list args
);
// Formatted output to the file
SUS_INLINE INT SUSAPIV sus_fscanfW(
	_In_ SUS_FILE hIn,
	_In_ LPCWSTR format,
	_Inout_ ...)
{
	sus_va_list args;
	sus_va_start(args, format);
	int res = sus_vfscanfW(hIn, format, args);
	sus_va_end(args);
	return res;
}

#ifdef UNICODE
#define sus_vfscanf		sus_vfscanfW
#define sus_fscanf		sus_fscanfW
#else
#define sus_vfscanf		sus_vfscanfA
#define sus_fscanf		sus_fscanfA
#endif // !UNICODE

// --------------------------------------------------------

// Formatted output to the console
SUS_INLINE INT SUSAPI sus_vprintfA(
	_In_ LPCSTR format,
	_In_ sus_va_list args) {
	return sus_vfprintfA(GetStdHandle(STD_OUTPUT_HANDLE), format, args);
}
// Formatted output to the console
SUS_INLINE INT SUSAPIV sus_printfA(
	_In_ LPCSTR format,
	_In_ ...)
{
	sus_va_list args;
	sus_va_start(args, format);
	int res = sus_vprintfA(format, args);
	sus_va_end(args);
	return res;
}
#define sus_printlnA(format, ...) sus_printfA(format "\n", __VA_ARGS__)

// Formatted output to the console
SUS_INLINE INT SUSAPI sus_vprintfW(
	_In_ LPCWSTR format,
	_In_ sus_va_list args) {
	return sus_vfprintfW(GetStdHandle(STD_OUTPUT_HANDLE), format, args);
}
// Formatted output to the console
SUS_INLINE INT SUSAPIV sus_printfW(
	_In_ LPCWSTR format,
	_In_ ...)
{
	sus_va_list args;
	sus_va_start(args, format);
	int res = sus_vprintfW(format, args);
	sus_va_end(args);
	return res;
}
#define sus_printlnW(format, ...) sus_printfW(format L"\n", __VA_ARGS__)

#ifdef UNICODE
#define sus_vprintf		sus_vprintfW
#define sus_printf		sus_printfW
#define sus_println		sus_printlnW
#else
#define sus_vprintf		sus_vprintfA
#define sus_printf		sus_printfA
#define sus_println		sus_printlnA
#endif // !UNICODE

// Formatted input to the console
SUS_INLINE INT SUSAPI sus_vscanfA(
	_In_ LPCSTR format,
	_Inout_ sus_va_list args) {
	return sus_vfscanfA(GetStdHandle(STD_INPUT_HANDLE), format, args);
}
// Formatted input to the console
SUS_INLINE INT SUSAPI sus_vscanfW(
	_In_ LPCWSTR format,
	_Inout_ sus_va_list args) {
	return sus_vfscanfW(GetStdHandle(STD_INPUT_HANDLE), format, args);
}

#ifdef UNICODE
#define sus_vscanf		sus_vscanfW
#else
#define sus_vscanf		sus_vscanfA
#endif // !UNICODE

// Formatted output to the console
SUS_INLINE INT SUSAPIV sus_scanfA(
	_In_ LPCSTR format,
	_Inout_ ...)
{
	sus_va_list args;
	sus_va_start(args, format);
	int res = sus_vscanfA(format, args);
	sus_va_end(args);
	return res;
}
// Formatted output to the console
SUS_INLINE INT SUSAPIV sus_scanfW(
	_In_ LPCWSTR format,
	_Inout_ ...)
{
	sus_va_list args;
	sus_va_start(args, format);
	int res = sus_vscanfW(format, args);
	sus_va_end(args);
	return res;
}

#ifdef UNICODE
#define sus_scanf		sus_scanfW
#else
#define sus_scanf		sus_scanfA
#endif // !UNICODE

// Ask to enter text in the console
SUS_INLINE INT SUSAPI sus_vinputA(_In_opt_ LPCSTR text, _In_ LPCSTR format, _Inout_ sus_va_list args) {
	if (text) sus_cwriteA(GetStdHandle(STD_OUTPUT_HANDLE), text, lstrlenA(text));
	int res = sus_vscanfA(format, args);
	return res;
}
// Ask to enter text in the console
SUS_INLINE INT SUSAPI sus_vinputW(_In_opt_ LPCWSTR text, _In_ LPCWSTR format, _Inout_ sus_va_list args) {
	if (text) sus_cwriteW(GetStdHandle(STD_OUTPUT_HANDLE), text, lstrlenW(text));
	int res = sus_vscanfW(format, args);
	return res;
}

#ifdef UNICODE
#define sus_vinput		sus_vinputW
#else
#define sus_vinput		sus_vinputA
#endif // !UNICODE

// Ask to enter text in the console
SUS_INLINE INT SUSAPI sus_inputA(_In_opt_ LPCSTR text, _In_ LPCSTR format, _Inout_ ...) {
	sus_va_list args;
	sus_va_start(args, format);
	int res = sus_vinputA(text, format, args);
	sus_va_end(args);
	return res;
}
// Ask to enter text in the console
SUS_INLINE INT SUSAPI sus_inputW(_In_opt_ LPCWSTR text, _In_ LPCWSTR format, _Inout_ ...) {
	sus_va_list args;
	sus_va_start(args, format);
	int res = sus_vinputW(text, format, args);
	sus_va_end(args);
	return res;
}

#ifdef UNICODE
#define sus_input		sus_inputW
#else
#define sus_input		sus_inputA
#endif // !UNICODE

// --------------------------------------------------------

#endif /* !_SUS_IO_STREAM_H_ */
