// string.c
//
#include "coreframe.h"
#include "include/susfwk/core.h"
#include "include/susfwk/string.h"

// -----------------------------------------------------------------------------

// Convert ascii format to unicode
VOID SUSAPI sus_atow(_In_ LPCSTR source, _Out_ LPWSTR buffer)
{
	SUS_ASSERT(source && buffer);
	while ((*buffer++ = (WCHAR)*source++));
}
// Convert Unicode format to Ascii
VOID SUSAPI sus_wtoa(_In_ LPCWSTR source, _Out_ LPSTR buffer)
{
	SUS_ASSERT(source && buffer);
	while ((*buffer++ = (CHAR)(*source++ <= 127 ? *(source - 1): '?')));
}

// Flipping the string
VOID SUSAPI sus_strrevA(_Inout_ LPSTR str)
{
	SUS_ASSERT(str);
	LPSTR end = str + lstrlenA(str) - 1;
	CHAR tmp;
	while (str < end) {
		tmp = *str;
		*str = *end;
		*end = tmp;
		str++;
		end--;
	}
}
// Flipping the string
VOID SUSAPI sus_strrevW(_Inout_ LPWSTR str)
{
	SUS_ASSERT(str);
	LPWSTR end = str + lstrlenW(str) - 1;
	WCHAR tmp;
	while (str < end) {
		tmp = *str;
		*str = *end;
		*end = tmp;
		str++;
		end--;
	}
}

// -----------------------------------------------------------------------------

// convert int to string
LPSTR SUSAPI sus_itoa(_Out_ LPSTR buffer, _In_ LONGLONG value)
{
	*buffer = '\0';
	LPSTR ptr = buffer;
	if (value < 0) {
		*ptr++ = '-';
		value = -value;
	}
	LPSTR start = ptr;
	do {
		*ptr++ = (value % 10) + '0';
		value /= 10;
	} while (value > 0);
	*ptr = '\0';
	LPSTR end = ptr - 1;
	while (start < end) {
		CHAR tmp = *start;
		*start++ = *end;
		*end-- = tmp;
	}
	return ptr;
}
// convert int to string
LPWSTR SUSAPI sus_itow(_Out_ LPWSTR buffer, _In_ LONGLONG value)
{
	*buffer = L'\0';
	LPWSTR ptr = buffer;
	if (value < 0) {
		*ptr++ = L'-';
		value = -value;
	}
	LPWSTR start = ptr;
	do {
		*ptr++ = (value % 10) + L'0';
		value /= 10;
	} while (value > 0);
	*ptr = L'\0';
	LPWSTR end = ptr - 1;
	while (start < end) {
		WCHAR tmp = *start;
		*start++ = *end;
		*end-- = tmp;
	}
	return ptr;
}
// convert float to string
LPSTR SUSAPI sus_ftoa(_Out_ LPSTR buffer, _In_ FLOAT value, _In_ DWORD precision)
{
	*buffer = '\0';
	LPSTR ptr = sus_itoa(buffer, (int)value);
	if (value < 0) value = -value;
	value -= (float)((int)value);
	*ptr++ = '.';
	for (DWORD i = 0; i < precision; i++) {
		value *= 10.0f;
		*ptr++ = (CHAR)((int)value + '0');
		value -= (FLOAT)((int)value);
		if (value < 0.001f) break;
	}
	*ptr = '\0';
	return ptr;
}
// convert float to string
LPWSTR SUSAPI sus_ftow(_Out_ LPWSTR buffer, _In_ FLOAT value, _In_ DWORD precision)
{
	*buffer = L'\0';
	LPWSTR ptr = sus_itow(buffer, (int)value);
	if (value < 0) value = -value;
	value -= (float)((int)value);
	*ptr++ = L'.';
	for (DWORD i = 0; i < precision; i++) {
		value *= 10.0f;
		*ptr++ = (WCHAR)((int)value + L'0');
		value -= (FLOAT)((int)value);
		if (value < 0.001f) break;
	}
	*ptr = L'\0';
	return ptr;
}

// convert string to int
LONGLONG SUSAPI sus_atoi(_In_ LPCSTR str, _Out_opt_ LPCSTR* end)
{
	LONGLONG value = 0;
	BOOLEAN isNegative = FALSE;
	while (*str == ' ') str++;
	if (*str == '-') {
		str++;
		isNegative = TRUE;
	}
	else if (*str == '+') str++;
	while (sus_isdigitA(*str)) {
		value = value * 10 + (LONGLONG)(*str++ - '0');
	}
	if (end) *end = str;
	return isNegative ? -value : value;
}
// convert string to int
LONGLONG SUSAPI sus_wtoi(_In_ LPCWSTR str, _Out_opt_ LPCWSTR* end)
{
	LONGLONG value = 0;
	BOOLEAN isNegative = FALSE;
	while (*str == L' ') str++;
	if (*str == L'-') {
		str++;
		isNegative = TRUE;
	}
	else if (*str == '+') str++;
	while (sus_isdigitW(*str)) {
		value = value * 10 + (LONGLONG)(*str++ - L'0');
	}
	if (end) *end = str;
	return isNegative ? -value : value;
}

// convert string to float
FLOAT SUSAPI sus_atof(_In_ LPCSTR str, _Out_opt_ LPCSTR* end)
{
	FLOAT value = (FLOAT)(INT)sus_atoi(str, &str);
	BOOLEAN isNegative = FALSE;
	if (value < 0.0f) {
		isNegative = TRUE; value = -value;
	}
	if (*str == '.') {
		str++;
		FLOAT decimalPlace = 1.0f;
		while (sus_isdigitA(*str)) {
			value += (*str++ - '0') * (decimalPlace /= 10.0f);
		}
	}
	if (end) *end = str;
	return isNegative ? -value : value;
}
// convert string to float
FLOAT SUSAPI sus_wtof(_In_ LPCWSTR str, _Out_opt_ LPCWSTR* end)
{
	FLOAT value = (FLOAT)(INT)sus_wtoi(str, &str);
	BOOLEAN isNegative = FALSE;
	if (value < 0.0f) {
		isNegative = TRUE; value = -value;
	}
	if (*str == L'.') {
		str++;
		FLOAT decimalPlace = 1.0f;
		while (sus_isdigitW(*str)) {
			value += (*str++ - L'0') * (decimalPlace /= 10.0f);
		}
	}
	if (end) *end = str;
	return isNegative ? -value : value;
}

// -----------------------------------------------------------------------------

// cropping a string
VOID SUSAPI sus_substringA(_Out_ LPSTR buffer, _In_ LPCSTR str, _In_ DWORD substart, _In_ DWORD subend)
{
	str += substart;
	while (substart++ < subend) {
		SUS_ASSERT(*str != '\0');
		*buffer++ = *str++;
	}
	*buffer = '\0';
}
// cropping a string
VOID SUSAPI sus_substringW(_Out_ LPWSTR buffer, _In_ LPCWSTR str, _In_ DWORD substart, _In_ DWORD subend)
{
	str += substart;
	while (substart++ < subend) {
		SUS_ASSERT(*str);
		*buffer++ = *str++;
	}
	*buffer = L'\0';
}

// Separation from spaces on the left
LPSTR SUSAPI sus_trimlA(_Inout_ LPSTR* str)
{
	SUS_ASSERT(str);
	while (sus_isspaceA(**str)) (*str)++;
	return *str;
}
// Separation from spaces on the left
LPWSTR SUSAPI sus_trimlW(_Inout_ LPWSTR* str)
{
	SUS_ASSERT(str);
	while (sus_isspaceW(**str)) (*str)++;
	return *str;
}
// Separation from spaces on the right
LPSTR SUSAPI sus_trimrA(_Inout_ LPSTR str)
{
	SUS_ASSERT(str);
	LPSTR end = str + lstrlenA(str);
	while (end > str && sus_isspaceA(*(end - 1))) end--;
	*end = '\0';
	return str;
}
// Separation from spaces on the right
LPWSTR SUSAPI sus_trimrW(_Inout_ LPWSTR str)
{
	SUS_ASSERT(str);
	LPWSTR end = str + lstrlenW(str);
	while (end > str && sus_isspaceW(*(end - 1))) end--;
	*end = L'\0';
	return str;
}
// Separation from spaces
LPSTR SUSAPI sus_trimA(_Inout_ LPSTR* str)
{
	SUS_ASSERT(str);
	return sus_trimrA(sus_trimlA(str));
}
// Separation from spaces
LPWSTR SUSAPI sus_trimW(_Inout_ LPWSTR* str)
{
	SUS_ASSERT(str);
	return sus_trimrW(sus_trimlW(str));
}

// -----------------------------------------------------------------------------

// The index of the symbol
LPSTR SUSAPI sus_strchrA(_In_ LPCSTR str, _In_ CHAR s)
{
	while (*str != s) if (!(*str++)) return NULL;
	return (LPSTR)str;
}
// The index of the symbol
LPWSTR SUSAPI sus_strchrW(_In_ LPCWSTR str, _In_ WCHAR s)
{
	while (*str != s) if (!(*str++)) return NULL;
	return (LPWSTR)str;
}
// Find a rock undergrowth in a row
LPSTR SUSAPI sus_strstrA(_In_ LPCSTR str, _In_ LPCSTR substring)
{
	if (!*substring) return (LPSTR)str;
	DWORD size = lstrlenA(substring) * sizeof(CHAR);
	for (; *str; str++) {
		if (!(str = sus_strchrA(str, *substring))) return NULL;
		if (sus_memcmp((CONST LPBYTE)str, (CONST LPBYTE)substring, size)) return (LPSTR)str;
	}
	return NULL;
}
// Find a rock undergrowth in a row
LPWSTR SUSAPI sus_strstrW(_In_ LPCWSTR str, _In_ LPCWSTR substring)
{
	if (!*substring) return (LPWSTR)str;
	DWORD size = lstrlenW(substring) * sizeof(WCHAR);
	for (; *str; str++) {
		if (!(str = sus_strchrW(str, *substring))) return NULL;
		if (sus_memcmp((CONST LPBYTE)str, (CONST LPBYTE)substring, size)) return (LPWSTR)str;
	}
	return NULL;
}

// The last index of the symbol
LPSTR SUSAPI sus_strrchrA(_In_ LPCSTR str, _In_ CHAR s)
{
	LPSTR cur = (LPSTR)str + lstrlenA(str) - 1;
	while (*cur != s) if (cur-- - str < 0) return NULL;
	return cur;
}
// The last index of the symbol
LPWSTR SUSAPI sus_strrchrW(_In_ LPCWSTR str, _In_ WCHAR s)
{
	LPWSTR cur = (LPWSTR)str + lstrlenW(str) - 1;
	while (*cur != s) if (cur-- - str < 0) return NULL;
	return cur;
}
// Find a rock undergrowth in a row
LPSTR SUSAPI sus_strrstrA(_In_ LPCSTR str, _In_ LPCSTR substring)
{
	if (!*substring) return (LPSTR)(str + lstrlenA(str));
	DWORD size = lstrlenA(substring) * sizeof(CHAR);
	for (LPSTR cur = (LPSTR)str + lstrlenA(str) - 1; cur >= str; cur--) {
		if (*cur == *substring && sus_memcmp((CONST LPBYTE)cur, (CONST LPBYTE)substring, size)) return cur;
	}
	return NULL;
}
// Find a rock undergrowth in a row
LPWSTR SUSAPI sus_strrstrW(_In_ LPCWSTR str, _In_ LPCWSTR substring)
{
	if (!*substring) return (LPWSTR)(str + lstrlenW(str));
	DWORD size = lstrlenW(substring) * sizeof(WCHAR);
	for (LPWSTR cur = (LPWSTR)str + lstrlenW(str) - 1; cur >= str; cur--) {
		if (*cur == *substring && sus_memcmp((CONST LPBYTE)cur, (CONST LPBYTE)substring, size)) return cur;
	}
	return NULL;
}

// parse a string for tokens
LPSTR SUSAPI sus_strtokA(_Inout_ LPSTR* ctx, _In_ LPCSTR delimiter)
{
	SUS_ASSERT(ctx && *ctx && delimiter);
	LPSTR token = *ctx;
	for (; *token && sus_strchrA(delimiter, *token); token++);
	if (!*token) return NULL;
	LPSTR separator = sus_strstrA(token, delimiter);
	if (separator) *separator = '\0';
	*ctx = (separator ? separator + lstrlenA(delimiter) : token + lstrlenA(token));
	return token;
}
// parse a string for tokens
LPWSTR SUSAPI sus_strtokW(_Inout_ LPWSTR* ctx, _In_ LPCWSTR delimiter)
{
	SUS_ASSERT(ctx && *ctx && delimiter);
	LPWSTR token = *ctx;
	for (; *token && sus_strchrW(delimiter, *token); token++);
	if (!*token) return NULL;
	LPWSTR separator = sus_strstrW(token, delimiter);
	if (separator) *separator = L'\0';
	*ctx = (separator ? separator + lstrlenW(delimiter) : token + lstrlenW(token));
	return token;
}

// -----------------------------------------------------------------------------
