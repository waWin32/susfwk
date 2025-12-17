// debug.h
//
#ifndef _SUS_DEBUG_
#define _SUS_DEBUG_

#ifdef _DEBUG
#define SUS_ASSERT(condition)	if (!(condition)) __debugbreak()

SUS_INLINE VOID SUSAPI SUS_DEBUG_PRINTF(LPCSTR format, ...) {
	sus_va_list args;
	sus_va_start(args, format);
	CHAR formattedString[512];
	wvsprintfA(formattedString, format, (va_list)args);
	OutputDebugStringA(formattedString);
	sus_va_end(args);
}
SUS_INLINE VOID SUSAPI SUS_DEBUG_WPRINTF(LPCWSTR format, ...) {
	sus_va_list args;
	sus_va_start(args, format);
	WCHAR formattedString[512];
	wvsprintfW(formattedString, format, (va_list)args);
	OutputDebugStringW(formattedString);
	sus_va_end(args);
}

#ifndef SUS_DEBUGONLYERRORS
#define SUS_PRINTDL(info, ...)		SUS_DEBUG_PRINTF("[" __FILE__ ":" TOSTRING(__LINE__) "] [info] " info "\n", __VA_ARGS__)
#define SUS_WPRINTDL(info, ...)		SUS_DEBUG_WPRINTF(L"[" WIDESTRING(__FILE__) L":" WIDESTRING(TOSTRING(__LINE__)) L"] [info] " L##info L"\n", __VA_ARGS__)
#define SUS_PRINTDW(warning, ...)	SUS_DEBUG_PRINTF("[" __FILE__ ":" TOSTRING(__LINE__) "] [warning] " warning "\n", __VA_ARGS__)
#define SUS_WPRINTDW(warning, ...)	SUS_DEBUG_WPRINTF(L"[" WIDESTRING(__FILE__) L":" WIDESTRING(TOSTRING(__LINE__)) L"] [warning] " L##warning L"\n", __VA_ARGS__)
#else
#define SUS_PRINTDL(debug, ...)
#define SUS_WPRINTDL(info, ...)
#define SUS_PRINTDW(debug, ...)
#define SUS_WPRINTDW(debug, ...)
#endif // !SUS_DEBUGONLYERRORS
#define SUS_PRINTDE(error, ...)		SUS_DEBUG_PRINTF("[" __FILE__ ":" TOSTRING(__LINE__) "] [error] " error "\n", __VA_ARGS__)
#define SUS_WPRINTDE(error, ...)	SUS_DEBUG_WPRINTF(L"[" WIDESTRING(__FILE__) L":" WIDESTRING(TOSTRING(__LINE__)) L"] [error] " L##error L"\n", __VA_ARGS__)
#define SUS_PRINTDC(code)			SUS_PRINTDE("Error code 0x%08X", (LONGLONG)code)
#define SUS_WPRINTDC(code)			SUS_WPRINTDE(L"Error code 0x%08X", (LONGLONG)code)
#else
#define SUS_ASSERT(condition)
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
