// format.c
//
#include "coreframe.h"
#include "include/susfwk/core.h"
#include "include/susfwk/string.h"
#include "include/susfwk/format.h"

static SUS_INLINE VOID SUSAPI format_handleA_d(_Inout_ LPSTR* buffer, _Inout_ LPINT length, _Inout_ sus_va_list* args, _In_ UINT width) {
	INT value = sus_va_arg(*args, INT);
	UINT len = sus_islen((INT64)value);
	INT padLen = (width > len) ? width - len : 0;
	if (*buffer) {
		if (width) {
			sus_memset((LPBYTE)*buffer, padLen, '0');
			*buffer += padLen;
		}
		*buffer = sus_itoa(*buffer, (INT64)value);
	}
	*length += padLen + len;
}
static SUS_INLINE VOID SUSAPI format_handleA_p(_Inout_ LPSTR* buffer, _Inout_ LPINT length, _Inout_ sus_va_list* args, _In_ UINT width) {
	SIZE_T value = sus_va_arg(*args, SIZE_T);
	UINT len = sus_islen((INT64)value);
	INT padLen = width > len ? width - len : 0;
	if (*buffer) {
		if (width) {
			sus_memset((LPBYTE)*buffer, padLen, '0');
			*buffer += padLen;
		}
		*buffer = sus_itoa(*buffer, (INT64)value);
	}
	*length += padLen + len;
}
static SUS_INLINE VOID SUSAPI format_handleA_f(_Inout_ LPSTR* buffer, _Inout_ LPINT length, _Inout_ sus_va_list* args, _In_ UINT width) {
	FLOAT value = sus_va_arg(*args, FLOAT);
	if (*buffer) {
		*buffer = sus_ftoa(*buffer, value, width);
	}
	*length += sus_fslen(value, width);
}
static SUS_INLINE VOID SUSAPI format_handleA_s(_Inout_ LPSTR* buffer, _Inout_ LPINT length, _Inout_ sus_va_list* args, _In_ UINT width) {
	LPSTR str = sus_va_arg(*args, LPSTR);
	if (str == NULL) str = "(null)";
	if (!width) width = lstrlenA(str);
	if (*buffer) {
		*buffer = lstrcpynA(*buffer, str, width + 1) + width;
	}
	*length += width;
}
static SUS_INLINE VOID SUSAPI format_handleA_c(_Inout_ LPSTR* buffer, _Inout_ LPINT length, _Inout_ sus_va_list* args) {
	CHAR ch = sus_va_arg(*args, CHAR);
	if (*buffer) {
		*(*buffer)++ = ch;
	}
	++(*length);
}

// text formatting
INT SUSAPI sus_vformattingA(
	_Out_opt_ LPSTR buffer,
	_In_ _Printf_format_string_ LPCSTR format,
	_In_ sus_va_list args)
{
	SUS_ASSERT(format);
	if (buffer) *buffer = '\0';
	INT length = 0;
	while (*format) {
		if (*format == '%') {
			format++;
			UINT width = 0;
			if (sus_isdigitA(*format)) width = (INT)sus_atoi(format, &format);
			switch (*(format++))
			{
			case 'd': {
				format_handleA_d(&buffer, &length, &args, width);
			} break;
			case 'f': {
				format_handleA_f(&buffer, &length, &args, width);
			} break;
			case 's': {
				format_handleA_s(&buffer, &length, &args, width);
			} break;
			case 'c': {
				format_handleA_c(&buffer, &length, &args);
			} break;
			case 'p': {
				format_handleA_p(&buffer, &length, &args, width);
			} break;
			default: {
				if (buffer) {
					*buffer++ = '%';
					*buffer++ = *(format-1);
				}
				format++;
				length++;
			}
			}
		}
		else {
			if (buffer) {
				*buffer++ = *format;
			}
			format++;
			length++;
		}
	}
	if (buffer) *buffer = '\0';
	return length;
}
// text formatting
INT SUSAPIV sus_formattingA(
	_Out_opt_ LPSTR buffer,
	_In_ _Printf_format_string_ LPCSTR format,
	_In_ ...)
{
	sus_va_list args;
	sus_va_start(args, format);
	SUS_ASSERT(format);
	INT res = sus_vformattingA(buffer, format, args);
	sus_va_end(args);
	return res;
}

static SUS_INLINE VOID SUSAPI format_handleW_d(_Inout_ LPWSTR* buffer, _Inout_ LPINT length, _Inout_ sus_va_list* args, _In_ UINT width) {
	INT value = sus_va_arg(*args, INT);
	UINT len = sus_islen((INT64)value);
	INT padLen = (width > len) ? width - len : 0;
	if (*buffer) {
		if (width) {
			sus_memset((LPBYTE)*buffer, padLen, L'0');
			*buffer += padLen;
		}
		*buffer = sus_itow(*buffer, (INT64)value);
	}
	*length += padLen + len;
}
static SUS_INLINE VOID SUSAPI format_handleW_p(_Inout_ LPWSTR* buffer, _Inout_ LPINT length, _Inout_ sus_va_list* args, _In_ UINT width) {
	SIZE_T value = sus_va_arg(*args, SIZE_T);
	UINT len = sus_islen((INT64)value);
	INT padLen = width > len ? width - len : 0;
	if (*buffer) {
		if (width) {
			sus_memset((LPBYTE)*buffer, padLen, L'0');
			*buffer += padLen;
		}
		*buffer = sus_itow(*buffer, (INT64)value);
	}
	*length += padLen + len;
}
static SUS_INLINE VOID SUSAPI format_handleW_f(_Inout_ LPWSTR* buffer, _Inout_ LPINT length, _Inout_ sus_va_list* args, _In_ UINT width) {
	FLOAT value = sus_va_arg(*args, FLOAT);
	if (*buffer) {
		*buffer = sus_ftow(*buffer, value, width);
	}
	*length += sus_fslen(value, width);
}
static SUS_INLINE VOID SUSAPI format_handleW_s(_Inout_ LPWSTR* buffer, _Inout_ LPINT length, _Inout_ sus_va_list* args, _In_ UINT width) {
	LPWSTR str = sus_va_arg(*args, LPWSTR);
	if (str == NULL) str = L"(null)";
	if (!width) width = lstrlenW(str);
	if (*buffer) {
		*buffer = lstrcpynW(*buffer, str, width + 1) + width;
	}
	*length += width;
}
static SUS_INLINE VOID SUSAPI format_handleW_c(_Inout_ LPWSTR* buffer, _Inout_ LPINT length, _Inout_ sus_va_list* args) {
	WCHAR ch = sus_va_arg(*args, WCHAR);
	if (*buffer) {
		*(*buffer)++ = ch;
	}
	++(*length);
}

// text formatting
INT SUSAPI sus_vformattingW(
	_Out_opt_ LPWSTR buffer,
	_In_ _Printf_format_string_ LPCWSTR format,
	_In_ sus_va_list args)
{
	SUS_ASSERT(format);
	if (buffer) *buffer = '\0';
	INT length = 0;
	while (*format) {
		if (*format == L'%') {
			format++;
			UINT width = 0;
			if (sus_isdigitW(*format)) width = (INT)sus_wtoi(format, &format);
			switch (*(format++))
			{
			case L'd': {
				format_handleW_d(&buffer, &length, &args, width);
			} break;
			case L'f': {
				format_handleW_f(&buffer, &length, &args, width);
			} break;
			case L's': {
				format_handleW_s(&buffer, &length, &args, width);
			} break;
			case L'c': {
				format_handleW_c(&buffer, &length, &args);
			} break;
			case L'p': {
				format_handleW_p(&buffer, &length, &args, width);
			} break;
			default: {
				if (buffer) {
					*buffer++ = L'%';
					*buffer++ = *(format - 1);
				}
				format++;
				length++;
			}
			}
		}
		else {
			if (buffer) {
				*buffer++ = *format;
			}
			format++;
			length++;
		}
	}
	if (buffer) *buffer = L'\0';
	return length;
}
// text formatting
INT SUSAPIV sus_formattingW(
	_Out_opt_ LPWSTR buffer,
	_In_ _Printf_format_string_ LPCWSTR format,
	_In_ ...)
{
	sus_va_list args;
	sus_va_start(args, format);   
	INT res = sus_vformattingW(buffer, format, args);
	sus_va_end(args);
	return res;
}

static SUS_INLINE BOOL SUSAPI parsing_handleA_d(_In_ BOOLEAN skipAssignment, _Inout_ LPCSTR* str, _Inout_ sus_va_list* args) {
	LPINT dest = skipAssignment ? NULL : sus_va_arg(*args, LPINT);
	LPSTR end = NULL;
	INT value = (INT)sus_atoi(*str, &end);
	if (*str == end) return FALSE;
	*str = end;
	if (dest) *dest = value;
	return TRUE;
}
static SUS_INLINE BOOL SUSAPI parsing_handleA_f(_In_ BOOLEAN skipAssignment, _Inout_ LPCSTR* str, _Inout_ sus_va_list* args) {
	PFLOAT dest = skipAssignment ? NULL : sus_va_arg(*args, PFLOAT);
	LPSTR end = NULL;
	FLOAT value = (FLOAT)sus_atof(*str, &end);
	if (*str == end) return FALSE;
	*str = end;
	if (dest) *dest = value;
	return TRUE;
}
static SUS_INLINE BOOL SUSAPI parsing_handleA_p(_In_ BOOLEAN skipAssignment, _Inout_ LPCSTR* str, _Inout_ sus_va_list* args) {
	SIZE_T* dest = skipAssignment ? NULL : sus_va_arg(*args, SIZE_T*);
	LPSTR end = NULL;
	SIZE_T value = (SIZE_T)sus_atoi(*str, &end);
	if (*str == end) return FALSE;
	*str = end;
	if (dest) *dest = value;
	return TRUE;
}
static SUS_INLINE BOOL SUSAPI parsing_handleA_s(_In_ BOOLEAN skipAssignment, _Inout_ LPCSTR* str, _Inout_ sus_va_list* args, _In_opt_ CHAR before, _In_ DWORD width) {
	LPSTR dest = skipAssignment ? NULL : sus_va_arg(*args, LPSTR);
	LPCSTR start = *str;
	while (**str != before && sus_isspaceA(**str)) {
		(*str)++;
	}
	if (dest) {
		while (**str != before && !sus_isspaceA(**str) && width > 0) {
			*dest++ = **str;
			(*str)++;
			width--;
		}
		*dest = '\0';
	}
	else {
		while (**str != before && !sus_isspaceA(**str) && width-- > 0) {
			(*str)++;
		}
	}
	return (*str != start);
}
static SUS_INLINE BOOL SUSAPI parsing_handleA_c(_In_ BOOLEAN skipAssignment, _Inout_ LPCSTR* str, _Inout_ sus_va_list* args) {
	PCHAR dest = skipAssignment ? NULL : sus_va_arg(*args, PCHAR);
	if (dest) *dest = **str;
	(*str)++;
	return TRUE;
}

// Parsing text into variables
INT sus_vparsingA(
	_In_ LPCSTR str,
	_In_ _Printf_format_string_ LPCSTR format,
	_Inout_ sus_va_list args)
{
	SUS_ASSERT(str && format);
	INT count = 0;
	DWORD width;
	LPCSTR start = str;
	while (*format && *str) {
		if (*format == '%') {
			format++;
			BOOLEAN skipAssignment = FALSE;
			BOOLEAN skipWhitespace = TRUE;
			while (TRUE) {
				switch (*format) {
				case '*': {
					skipAssignment = TRUE;
					format++;
				} continue;
				case ' ': {
					skipWhitespace = FALSE;
					format++;
				} continue;
				}
				break;
			}
			if (sus_isdigitA(*format)) width = (INT)sus_atoi(format, &format);
			else width = INFINITE;
			if (skipWhitespace) while (sus_isspaceA(*str)) str++;
			switch (*(format++))
			{
			case 'd': {
				if (!parsing_handleA_d(skipAssignment, &str, &args)) return count;
			} break;
			case 'p': {
				if (!parsing_handleA_p(skipAssignment, &str, &args)) return count;
			} break;
			case 'f': {
				if (!parsing_handleA_f(skipAssignment, &str, &args)) return count;
			} break;
			case '^': {
				if (!parsing_handleA_s(skipAssignment, &str, &args, *str++, width)) return count;
			} break;
			case 's': {
				if (!parsing_handleA_s(skipAssignment, &str, &args, '\0', width)) return count;
			} break;
			case 'c': {
				if (!parsing_handleA_c(skipAssignment, &str, &args)) return count;
			} break;
			case 'n': {
				LPINT dest = skipAssignment ? NULL : sus_va_arg(args, LPINT);
				if (dest) *dest = (INT)(str - start);
				format++;
				continue;
			}
			default: {
				if (*str++ != *(format - 1)) return count;
				continue;
			}
			}
			format++;
			if (!skipAssignment) count++;
		}
		else if (*str++ != *format++) break;
	}
	return count;
}
// Parsing text into variables
INT sus_parsingA(
	_In_ LPCSTR str,
	_In_ _Printf_format_string_ LPCSTR format,
	_Out_ ...)
{
	sus_va_list args;
	sus_va_start(args, format);
	INT res = sus_vparsingA(str, format, args);
	sus_va_end(args);
	return res;
}


static SUS_INLINE BOOL SUSAPI parsing_handleW_d(_In_ BOOLEAN skipAssignment, _Inout_ LPCWSTR* str, _Inout_ sus_va_list* args) {
	LPINT dest = skipAssignment ? NULL : sus_va_arg(*args, LPINT);
	LPWSTR end = NULL;
	INT value = (INT)sus_wtoi(*str, &end);
	if (*str == end) return FALSE;
	*str = end;
	if (dest) *dest = value;
	return TRUE;
}
static SUS_INLINE BOOL SUSAPI parsing_handleW_f(_In_ BOOLEAN skipAssignment, _Inout_ LPCWSTR* str, _Inout_ sus_va_list* args) {
	PFLOAT dest = skipAssignment ? NULL : sus_va_arg(*args, PFLOAT);
	LPWSTR end = NULL;
	FLOAT value = (FLOAT)sus_wtof(*str, &end);
	if (*str == end) return FALSE;
	*str = end;
	if (dest) *dest = value;
	return TRUE;
}
static SUS_INLINE BOOL SUSAPI parsing_handleW_p(_In_ BOOLEAN skipAssignment, _Inout_ LPCWSTR* str, _Inout_ sus_va_list* args) {
	SIZE_T* dest = skipAssignment ? NULL : sus_va_arg(*args, SIZE_T*);
	LPWSTR end = NULL;
	SIZE_T value = (SIZE_T)sus_wtoi(*str, &end);
	if (*str == end) return FALSE;
	*str = end;
	if (dest) *dest = value;
	return TRUE;
}
static SUS_INLINE BOOL SUSAPI parsing_handleW_s(_In_ BOOLEAN skipAssignment, _Inout_ LPCWSTR* str, _Inout_ sus_va_list* args, _In_opt_ WCHAR before, _In_ DWORD width) {
	LPWSTR dest = skipAssignment ? NULL : sus_va_arg(*args, LPWSTR);
	LPCWSTR start = *str;
	while (**str != before && sus_isspaceW(**str)) {
		(*str)++;
	}
	if (dest) {
		while (**str != before && !sus_isspaceW(**str) && width > 0) {
			*dest++ = **str;
			(*str)++;
			width--;
		}
		*dest = L'\0';
	}
	else {
		while (**str != before && !sus_isspaceW(**str) && width-- > 0) {
			(*str)++;
		}
	}
	return (*str != start);
}
static SUS_INLINE BOOL SUSAPI parsing_handleW_c(_In_ BOOLEAN skipAssignment, _Inout_ LPCWSTR* str, _Inout_ sus_va_list* args) {
	PWCHAR dest = skipAssignment ? NULL : sus_va_arg(*args, PWCHAR);
	if (dest) *dest = **str;
	(*str)++;
	return TRUE;
}

// Parsing text into variables
INT sus_vparsingW(
	_In_ LPCWSTR str,
	_In_ _Printf_format_string_ LPCWSTR format,
	_Inout_ sus_va_list args)
{
	SUS_ASSERT(str && format);
	INT count = 0;
	DWORD width;
	LPCWSTR start = str;
	while (*format && *str) {
		if (*format == L'%') {
			format++;
			BOOLEAN skipAssignment = FALSE;
			BOOLEAN skipWhitespace = TRUE;
			while (TRUE) {
				switch (*format) {
				case L'*': {
					skipAssignment = TRUE;
					format++;
				} continue;
				case L' ': {
					skipWhitespace = FALSE;
					format++;
				} continue;
				}
				break;
			}
			if (sus_isdigitW(*format)) width = (INT)sus_wtoi(format, &format);
			else width = INFINITE;
			if (skipWhitespace) while (sus_isspaceW(*str)) str++;
			switch (*(format++))
			{
			case L'd': {
				if (!parsing_handleW_d(skipAssignment, &str, &args)) return count;
			} break;
			case L'p': {
				if (!parsing_handleW_p(skipAssignment, &str, &args)) return count;
			} break;
			case L'f': {
				if (!parsing_handleW_f(skipAssignment, &str, &args)) return count;
			} break;
			case L'^': {
				if (!parsing_handleW_s(skipAssignment, &str, &args, *str++, width)) return count;
			} break;
			case L's': {
				if (!parsing_handleW_s(skipAssignment, &str, &args, L'\0', width)) return count;
			} break;
			case L'c': {
				if (!parsing_handleW_c(skipAssignment, &str, &args)) return count;
			} break;
			case L'n': {
				LPINT dest = skipAssignment ? NULL : sus_va_arg(args, LPINT);
				if (dest) *dest = (INT)(str - start);
				format++;
				continue;
			}
			default: {
				if (*str++ != *(format - 1)) return count;
				continue;
			}
			}
			format++;
			if (!skipAssignment) count++;
		}
		else if (*str++ != *format++) break;
	}
	return count;
}
// Parsing text into variables
INT sus_parsingW(
	_In_ LPCWSTR str,
	_In_ _Printf_format_string_ LPCWSTR format,
	_Out_ ...)
{
	sus_va_list args;
	sus_va_start(args, format);
	INT res = sus_vparsingW(str, format, args);
	sus_va_end(args);
	return res;
}

// parsing a string with special characters in the esc sequence
DWORD SUSAPI sus_escapeA(_Out_opt_ LPSTR buff, _In_ LPCSTR src)
{
	DWORD length = 0;
	while (*src) {
		if (*src < 32 || *src == '"' || *src == '\\') {
			if (buff) *buff++ = '\\';
			switch (*src++)
			{
			case '\"': if (buff) *buff++ = '"'; break;
			case '\\': if (buff) *buff++ = '\\'; break;
			case '\t': if (buff) *buff++ = 't'; break;
			case '\n': if (buff) *buff++ = 'n'; break;
			case '\v': if (buff) *buff++ = 'v'; break;
			case '\r': if (buff) *buff++ = 'r'; break;
			case '\f': if (buff) *buff++ = 'f'; break;
			case '\a': if (buff) *buff++ = 'a'; break;
			case '\b': if (buff) *buff++ = 'b'; break;
			case '\0': if (buff) *buff++ = '0'; break;
			default: {
				*(buff - 1) = *(src - 1);
				length--;
			} break;
			}
			length += 2;
		}
		else {
			if (buff) *buff++ = *src++;
			else src++;
			length++;
		}
	}
	if (buff) *buff = '\0';
	return length;
}
// parsing a string with special characters in the esc sequence
DWORD SUSAPI sus_escapeW(_Out_opt_ LPWSTR buff, _In_ LPCWSTR src)
{
	DWORD length = 0;
	while (*src) {
		if (*src < 32 || *src == L'"' || *src == L'\\') {
			if (buff) *buff++ = L'\\';
			switch (*src++)
			{
			case L'\"': if (buff) *buff++ = L'"'; break;
			case L'\\': if (buff) *buff++ = L'\\'; break;
			case L'\t': if (buff) *buff++ = L't'; break;
			case L'\n': if (buff) *buff++ = L'n'; break;
			case L'\v': if (buff) *buff++ = L'v'; break;
			case L'\r': if (buff) *buff++ = L'r'; break;
			case L'\f': if (buff) *buff++ = L'f'; break;
			case L'\a': if (buff) *buff++ = L'a'; break;
			case L'\b': if (buff) *buff++ = L'b'; break;
			case L'\0': if (buff) *buff++ = L'0'; break;
			default: {
				*(buff - 1) = *(src - 1);
				length--;
			} break;
			}
			length += 2;
		}
		else {
			if (buff) *buff++ = *src++;
			else src++;
			length++;
		}
	}
	if (buff) *buff = L'\0';
	return length;
}
// parsing a string with esc characters into special characters
DWORD SUSAPI sus_unescapeA(_Out_opt_ LPSTR buff, _In_ LPCSTR src)
{
	DWORD length = 0;
	while (*src) {
		if (*src == '\\') {
			src++;
			if (!*src) break;
			switch (*(src++))
			{
			case '"': if (buff) *buff++ = '\"'; break;
			case 't': if (buff) *buff++ = '\t'; break;
			case 'n': if (buff) *buff++ = '\n'; break;
			case 'v': if (buff) *buff++ = '\v'; break;
			case 'r': if (buff) *buff++ = '\r'; break;
			case 'f': if (buff) *buff++ = '\f'; break;
			case 'a': if (buff) *buff++ = '\a'; break;
			case 'b': if (buff) *buff++ = '\b'; break;
			case '0': if (buff) *buff++ = '\0'; break;
			case '\\': if (buff) *buff++ = '\\'; break;
			default: {
				if (buff) {
					*buff++ = '\\';
					*buff++ = *(src - 1);
				}
				length += 1;
			} break;
			}
			length++;
		}
		else {
			if (buff) *buff++ = *src++;
			else src++;
			length++;
		}
	}
	if (buff) *buff = '\0';
	return length;
}
// parsing a string with esc characters into special characters
DWORD SUSAPI sus_unescapeW(_Out_opt_ LPWSTR buff, _In_ LPCWSTR src)
{
	DWORD length = 0;
	while (*src) {
		if (*src == L'\\') {
			src++;
			if (!*src) break;
			switch (*(src++))
			{
			case L'"': if (buff) *buff++ = L'\"'; break;
			case L't': if (buff) *buff++ = L'\t'; break;
			case L'n': if (buff) *buff++ = L'\n'; break;
			case L'v': if (buff) *buff++ = L'\v'; break;
			case L'r': if (buff) *buff++ = L'\r'; break;
			case L'f': if (buff) *buff++ = L'\f'; break;
			case L'a': if (buff) *buff++ = L'\a'; break;
			case L'b': if (buff) *buff++ = L'\b'; break;
			case L'0': if (buff) *buff++ = L'\0'; break;
			case L'\\': if (buff) *buff++ = L'\\'; break;
			default: {
				if (buff) {
					*buff++ = L'\\';
					*buff++ = *(src - 1);
				}
				length += 1;
			} break;
			}
			length++;
		}
		else {
			if (buff) *buff++ = *src++;
			else src++;
			length++;
		}
	}
	if (buff) *buff = L'\0';
	return length;
}
