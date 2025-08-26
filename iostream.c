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
	CHAR buff[1024];
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
	WCHAR buff[1024];
	buff[sus_readW(hIn, buff, (sizeof(buff) - 1) / sizeof(WCHAR))] = L'\0';
	return sus_vparsingW(buff, format, args);
}

// --------------------------------------------------------
