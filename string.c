// string.c
//
#include "coreframe.h"
#include "include/susfwk/core.h"
#include "include/susfwk/string.h"

// -----------------------------------------------------------------------------

// Flipping the string
VOID SUSAPI sus_strrevA(_Inout_ LPSTR str)
{
	SUS_ASSERT(str);
	LPSTR end = str + sus_strlenA(str) - 1;
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
	LPWSTR end = str + sus_strlenW(str) - 1;
	WCHAR tmp;
	while (str < end) {
		tmp = *str;
		*str = *end;
		*end = tmp;
		str++;
		end--;
	}
}
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
	*ptr++ = '.';
	value -= (float)((int)value);
	for (DWORD i = 0; i < precision; i++) {
		value *= 10.0f;
		*ptr++ = (CHAR)((int)value + '0');
		value -= (FLOAT)((int)value);
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
	*ptr++ = L'.';
	value -= (float)((int)value);
	for (DWORD i = 0; i < precision; i++) {
		value *= 10.0f;
		*ptr++ = (WCHAR)((int)value + L'0');
		value -= (FLOAT)((int)value);
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
		value = value * 10 + (*str++ - '0');
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
		value = value * 10 + (*str++ - L'0');
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
VOID SUSAPI sus_substringA(
	_Out_ LPSTR buffer,
	_In_ LPCSTR str,
	_In_ DWORD substart,
	_In_ DWORD subend)
{
	str += substart;
	while (substart++ < subend) {
		SUS_ASSERT(*str != '\0');
		*buffer++ = *str++;
	}
	*buffer = '\0';
}
// cropping a string
VOID SUSAPI sus_substringW(
	_Out_ LPWSTR buffer,
	_In_ LPCWSTR str,
	_In_ DWORD substart,
	_In_ DWORD subend)
{
	str += substart;
	while (substart++ < subend) {
		SUS_ASSERT(*str);
		*buffer++ = *str++;
	}
	*buffer = L'\0';
}
// The index of the symbol
LPSTR SUSAPI sus_strchrA(
	_In_ LPCSTR str,
	_In_ CHAR s)
{
	while (*str != s) if (!(*(LPSTR)str++)) return NULL;
	return (LPSTR)str;
}
// The index of the symbol
LPWSTR SUSAPI sus_strchrW(
	_In_ LPCWSTR str,
	_In_ WCHAR s)
{
	while (*str != s) if (!(*(LPWSTR)str++)) return NULL;
	return (LPWSTR)str;
}
// The last index of the symbol
LPSTR SUSAPI sus_strrchrA(
	_In_ LPCSTR str,
	_In_ CHAR s)
{
	LPSTR cur = (LPSTR)str + lstrlenA(str) - 1;
	while (*cur != s) if (cur-- - str < 0) return NULL;
	return cur;
}
// The last index of the symbol
LPWSTR SUSAPI sus_strrchrW(
	_In_ LPCWSTR str,
	_In_ WCHAR s)
{
	LPWSTR cur = (LPWSTR)str + lstrlenW(str) - 1;
	while (*cur != s) if (cur-- - str < 0) return NULL;
	return cur;
}

// -----------------------------------------------------------------------------
