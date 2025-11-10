// iostream.c
//
#include "coreframe.h"
#include "include/susfwk/core.h"
#include "include/susfwk/string.h"
#include "include/susfwk/format.h"
#include "include/susfwk/memory.h"
#include "include/susfwk/fileio.h"
#include "include/susfwk/conio.h"
#include "include/susfwk/iostream.h"

// --------------------------------------------------------

// Formatted output to the file
INT SUSAPI sus_vfprintfA(
	_In_ SUS_FILE hOut,
	_In_ LPCSTR format,
	_In_ sus_va_list args)
{
	INT len = sus_vformattingA(NULL, format, args);
	LPSTR lpFormattedString = sus_fmalloc(((SIZE_T)len + 1) * sizeof(CHAR));
	if (!lpFormattedString) return -1;
	sus_vformattingA(lpFormattedString, format, args);
	sus_writeA(hOut, lpFormattedString, len);
	sus_free(lpFormattedString);
	return len;
}
// Formatted output to the file
INT SUSAPI sus_vfprintfW(
	_In_ SUS_FILE hOut,
	_In_ LPCWSTR format,
	_In_ sus_va_list args)
{
	INT len = sus_vformattingW(NULL, format, args);
	LPWSTR lpFormattedString = sus_fmalloc(((SIZE_T)len + 1) * sizeof(WCHAR));
	if (!lpFormattedString) return -1;
	sus_vformattingW(lpFormattedString, format, args);
	sus_writeW(hOut, lpFormattedString, len);
	sus_free(lpFormattedString);
	return len;
}

// --------------------------------------------------------

// Formatted input to the file
INT SUSAPI sus_vfscanfA(
	_In_ SUS_FILE hIn,
	_In_ LPCSTR format,
	_Inout_ sus_va_list args)
{
	SUS_ASSERT(hIn && format && args);
	CHAR buff[1024] = { 0 };
	buff[sus_readA(hIn, buff, (sizeof(buff) - 1) / sizeof(CHAR))] = '\0';
	return sus_vparsingA(buff, format, args);
}
// Formatted input to the file
INT SUSAPI sus_vfscanfW(
	_In_ SUS_FILE hIn,
	_In_ LPCWSTR format,
	_Inout_ sus_va_list args)
{
	SUS_ASSERT(hIn && format && args);
	WCHAR buff[1024] = { 0 };
	buff[sus_readW(hIn, buff, (sizeof(buff) - 1) / sizeof(WCHAR))] = L'\0';
	return sus_vparsingW(buff, format, args);
}

// --------------------------------------------------------

// output a message box with a format string
INT SUSAPIV susMessageBoxA(_In_opt_ HWND hWnd, _In_ LPCSTR ftext, _In_ LPCSTR header, _In_ UINT uType, ...)
{
	sus_va_list list;
	sus_va_start(list, uType);
	LPSTR text = ftext ? sus_dvformattingA(ftext, list) : NULL;
	INT rez = MessageBoxA(hWnd, text ? text : "null", header, uType);
	if (text) sus_strfree(text);
	return rez;
}
// output a message box with a format string
INT SUSAPIV susMessageBoxW(_In_opt_ HWND hWnd, _In_ LPCWSTR ftext, _In_ LPCWSTR header, _In_ UINT uType, ...)
{
	sus_va_list list;
	sus_va_start(list, uType);
	LPWSTR text = ftext ? sus_dvformattingW(ftext, list) : NULL;
	INT rez = MessageBoxW(hWnd, text ? text : L"null", header, uType);
	if (text) sus_wcsfree(text);
	return rez;
}

// --------------------------------------------------------
