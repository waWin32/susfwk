// debug.h
//
#ifndef _SUS_DEBUG_
#define _SUS_DEBUG_

#ifdef _DEBUG
#include "format.h"
#define SUS_ASSERT(condition)	if (!(condition)) __debugbreak()
SUS_INLINE VOID SUSAPI _SUS_VDEBUG_PRINT(SUS_FILE hOut, WORD wColor, LPCSTR format, sus_va_list args) {
	if (!hOut) return;
	CHAR formattedString[256];
	sus_vformattingA(formattedString, format, args);
	DWORD mode;
	if (GetConsoleMode(hOut, &mode)) {
		if (wColor != 7) SetConsoleTextAttribute(hOut, wColor);
		WriteConsoleA(hOut, formattedString, lstrlenA(formattedString), NULL, NULL);
		if (wColor != 7) SetConsoleTextAttribute(hOut, 7);
	}
	else {
		WriteFile(hOut, formattedString, lstrlenA(formattedString) * sizeof(CHAR), NULL, NULL);
	}
}
SUS_INLINE VOID SUSAPI _SUS_DEBUG_PRINTLOG(SUS_FILE hOut, LPCSTR format, ...) {
	sus_va_list args;
	sus_va_start(args, format);
	_SUS_VDEBUG_PRINT(hOut, 7, format, args);
	sus_va_end(args);
}
SUS_INLINE VOID SUSAPI _SUS_DEBUG_PRINTWAR(SUS_FILE hOut, LPCSTR format, ...) {
	sus_va_list args;
	sus_va_start(args, format);
	_SUS_VDEBUG_PRINT(hOut, 6, format, args);
	sus_va_end(args);
}
SUS_INLINE VOID SUSAPI _SUS_DEBUG_PRINTERR(SUS_FILE hOut, LPCSTR format, ...) {
	sus_va_list args;
	sus_va_start(args, format);
	_SUS_VDEBUG_PRINT(hOut, 4, format, args);
	sus_va_end(args);
}

static SUS_FILE hDebugOut;

SUS_INLINE VOID SUSAPI SUS_CONSOLE_DEBUGGING() { AllocConsole(); hDebugOut = GetStdHandle(STD_OUTPUT_HANDLE); }
SUS_INLINE VOID SUSAPI SUS_LOGFILE_DEBUGGING(SUS_FILE logFile) { hDebugOut = logFile; }
SUS_INLINE VOID SUSAPI SUS_CLOSE_LOGFILE() { CloseHandle(hDebugOut); }

#define SUS_PRINTDL(info, ...)		_SUS_DEBUG_PRINTLOG(hDebugOut, "[" __FILE__ ":" TOSTRING(__LINE__) "] [info] " info "\n", __VA_ARGS__)
#define SUS_PRINTDE(error, ...)		_SUS_DEBUG_PRINTERR(hDebugOut, "[" __FILE__ ":" TOSTRING(__LINE__) "] [error] " error "\n", __VA_ARGS__)
#define SUS_PRINTDW(warning, ...)	_SUS_DEBUG_PRINTWAR(hDebugOut, "[" __FILE__ ":" TOSTRING(__LINE__) "] [warning] " warning "\n", __VA_ARGS__)
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
