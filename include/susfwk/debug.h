// debug.h
//
#ifndef _SUS_DEBUG_
#define _SUS_DEBUG_

#ifdef _DEBUG
#include "format.h"
#define SUS_ASSERT(condition)	if (!(condition)) __debugbreak()

SUS_FILE hDebugOut;

SUS_INLINE VOID SUSAPI _SUS_VDEBUG_PRINT(WORD wColor, LPCSTR format, sus_va_list args) {
	if (!hDebugOut) return;
	CHAR formattedString[512];
	sus_vformattingA(formattedString, format, args);
	DWORD mode;
	if (GetConsoleMode(hDebugOut, &mode)) {
		if (wColor != 2) SetConsoleTextAttribute(hDebugOut, wColor);
		WriteConsoleA(hDebugOut, formattedString, lstrlenA(formattedString), NULL, NULL);
		if (wColor != 2) SetConsoleTextAttribute(hDebugOut, 2);
	}
	else {
		WriteFile(hDebugOut, formattedString, lstrlenA(formattedString) * sizeof(CHAR), NULL, NULL);
	}
}
SUS_INLINE VOID SUSAPI _SUS_VDEBUG_WPRINT(WORD wColor, LPCWSTR format, sus_va_list args) {
	if (!hDebugOut) return;
	WCHAR formattedString[512];
	sus_vformattingW(formattedString, format, args);
	DWORD mode;
	if (GetConsoleMode(hDebugOut, &mode)) {
		SetConsoleTextAttribute(hDebugOut, wColor);
		WriteConsoleW(hDebugOut, formattedString, lstrlenW(formattedString), NULL, NULL);
		if (wColor != 2) SetConsoleTextAttribute(hDebugOut, 2);
	}
	else {
		WriteFile(hDebugOut, formattedString, lstrlenW(formattedString) * sizeof(WCHAR), NULL, NULL);
	}
}
SUS_INLINE VOID SUSAPI _SUS_DEBUG_PRINTLOG(LPCSTR format, ...) {
	sus_va_list args;
	sus_va_start(args, format);
	_SUS_VDEBUG_PRINT(2, format, args);
	sus_va_end(args);
}
SUS_INLINE VOID SUSAPI _SUS_DEBUG_WPRINTLOG(LPCWSTR format, ...) {
	sus_va_list args;
	sus_va_start(args, format);
	_SUS_VDEBUG_WPRINT(2, format, args);
	sus_va_end(args);
}
SUS_INLINE VOID SUSAPI _SUS_DEBUG_PRINTWAR(LPCSTR format, ...) {
	sus_va_list args;
	sus_va_start(args, format);
	_SUS_VDEBUG_PRINT(6, format, args);
	sus_va_end(args);
}
SUS_INLINE VOID SUSAPI _SUS_DEBUG_WPRINTWAR(LPCWSTR format, ...) {
	sus_va_list args;
	sus_va_start(args, format);
	_SUS_VDEBUG_WPRINT(6, format, args);
	sus_va_end(args);
}
SUS_INLINE VOID SUSAPI _SUS_DEBUG_PRINTERR(LPCSTR format, ...) {
	sus_va_list args;
	sus_va_start(args, format);
	_SUS_VDEBUG_PRINT(4, format, args);
	sus_va_end(args);
}
SUS_INLINE VOID SUSAPI _SUS_DEBUG_WPRINTERR(LPCWSTR format, ...) {
	sus_va_list args;
	sus_va_start(args, format);
	_SUS_VDEBUG_WPRINT(4, format, args);
	sus_va_end(args);
}

SUS_INLINE VOID SUSAPI SUS_CONSOLE_DEBUGGING() { AllocConsole(); hDebugOut = GetStdHandle(STD_OUTPUT_HANDLE); }
SUS_INLINE VOID SUSAPI SUS_LOGFILE_DEBUGGING(SUS_FILE logFile) { hDebugOut = logFile; }
SUS_INLINE VOID SUSAPI SUS_CLOSE_LOGFILE() { CloseHandle(hDebugOut); }

#ifndef SUS_DEBUGONLYERRORS
#define SUS_PRINTDL(info, ...)		_SUS_DEBUG_PRINTLOG("[" __FILE__ ":" TOSTRING(__LINE__) "] [info] " info "\n", __VA_ARGS__)
#define SUS_WPRINTDL(info, ...)		_SUS_DEBUG_WPRINTLOG(L"[" WIDESTRING(__FILE__) L":" WIDESTRING(TOSTRING(__LINE__)) L"] [info] " L##info L"\n", __VA_ARGS__)
#define SUS_PRINTDW(warning, ...)	_SUS_DEBUG_PRINTWAR("[" __FILE__ ":" TOSTRING(__LINE__) "] [warning] " warning "\n", __VA_ARGS__)
#define SUS_WPRINTDW(warning, ...)	_SUS_DEBUG_WPRINTWAR(L"[" WIDESTRING(__FILE__) L":" WIDESTRING(TOSTRING(__LINE__)) L"] [warning] " L##warning L"\n", __VA_ARGS__)
#else
#define SUS_PRINTDL(debug, ...)
#define SUS_WPRINTDL(info, ...)
#define SUS_PRINTDW(debug, ...)
#define SUS_WPRINTDW(debug, ...)
#endif // !SUS_DEBUGONLYERRORS
#define SUS_PRINTDE(error, ...)		_SUS_DEBUG_PRINTERR("[" __FILE__ ":" TOSTRING(__LINE__) "] [error] " error "\n", __VA_ARGS__)
#define SUS_WPRINTDE(error, ...)	_SUS_DEBUG_WPRINTERR(L"[" WIDESTRING(__FILE__) L":" WIDESTRING(TOSTRING(__LINE__)) L"] [error] " L##error L"\n", __VA_ARGS__)
#define SUS_PRINTDC(code)			SUS_PRINTDE("Error code %p", (LONGLONG)code)
#define SUS_WPRINTDC(code)			SUS_WPRINTDE(L"Error code %p", (LONGLONG)code)
#else
#define SUS_ASSERT(condition)
#define SUS_SET_DEBUG_OUT(hOut)
#define SUS_CONSOLE_DEBUGGING()
#define SUS_LOGFILE_DEBUGGING(logFile)
#define SUS_CLOSE_LOGFILE()
#define SUS_PRINTDL(debug, ...)
#define SUS_WPRINTDL(info, ...)
#define SUS_PRINTDE(debug, ...)
#define SUS_WPRINTDE(debug, ...)
#define SUS_PRINTDW(debug, ...)
#define SUS_WPRINTDW(debug, ...)
#define SUS_PRINTDC(code)
#define SUS_WPRINTDC(code)
#endif // !_DEBUG

#endif /* !_SUS_DEBUG_ */
