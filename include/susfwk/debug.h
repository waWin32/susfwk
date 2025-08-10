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
		if (wColor != 7) SetConsoleTextAttribute(hDebugOut, wColor);
		WriteConsoleA(hDebugOut, formattedString, lstrlenA(formattedString), NULL, NULL);
		if (wColor != 7) SetConsoleTextAttribute(hDebugOut, 7);
	}
	else {
		WriteFile(hDebugOut, formattedString, lstrlenA(formattedString) * sizeof(CHAR), NULL, NULL);
	}
}
SUS_INLINE VOID SUSAPI _SUS_DEBUG_PRINTLOG(LPCSTR format, ...) {
	sus_va_list args;
	sus_va_start(args, format);
	_SUS_VDEBUG_PRINT(7, format, args);
	sus_va_end(args);
}
SUS_INLINE VOID SUSAPI _SUS_DEBUG_PRINTWAR(LPCSTR format, ...) {
	sus_va_list args;
	sus_va_start(args, format);
	_SUS_VDEBUG_PRINT(6, format, args);
	sus_va_end(args);
}
SUS_INLINE VOID SUSAPI _SUS_DEBUG_PRINTERR(LPCSTR format, ...) {
	sus_va_list args;
	sus_va_start(args, format);
	_SUS_VDEBUG_PRINT(4, format, args);
	sus_va_end(args);
}

SUS_INLINE VOID SUSAPI SUS_CONSOLE_DEBUGGING() { AllocConsole(); hDebugOut = GetStdHandle(STD_OUTPUT_HANDLE); }
SUS_INLINE VOID SUSAPI SUS_LOGFILE_DEBUGGING(SUS_FILE logFile) { hDebugOut = logFile; }
SUS_INLINE VOID SUSAPI SUS_CLOSE_LOGFILE() { CloseHandle(hDebugOut); }

#ifndef SUS_DEBUGONLYERRORS
#define SUS_PRINTDL(info, ...)		_SUS_DEBUG_PRINTLOG("[" __FILE__ ":" TOSTRING(__LINE__) "] [info] " info "\n", __VA_ARGS__)
#else
#define SUS_PRINTDL(debug, ...)
#endif // !SUS_DEBUGONLYERRORS
#define SUS_PRINTDE(error, ...)		_SUS_DEBUG_PRINTERR("[" __FILE__ ":" TOSTRING(__LINE__) "] [error] " error "\n", __VA_ARGS__)
#define SUS_PRINTDW(warning, ...)	_SUS_DEBUG_PRINTWAR("[" __FILE__ ":" TOSTRING(__LINE__) "] [warning] " warning "\n", __VA_ARGS__)
#define SUS_PRINTDC(code)			SUS_PRINTDE("Error code %p", (LONGLONG)code)
#else
#define SUS_ASSERT(condition)
#define SUS_SET_DEBUG_OUT(hOut)
#define SUS_CONSOLE_DEBUGGING()
#define SUS_LOGFILE_DEBUGGING(logFile)
#define SUS_CLOSE_LOGFILE()
#define SUS_PRINTDL(debug, ...)
#define SUS_PRINTDE(debug, ...)
#define SUS_PRINTDW(debug, ...)
#define SUS_PRINTDC(code)
#endif // !_DEBUG

#endif /* !_SUS_DEBUG_ */
