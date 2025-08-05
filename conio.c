// conio.c
//
#include "coreframe.h"
#include "include/susfwk/core.h"
#include "include/susfwk/conio.h"

// Write text to the console
INT SUSAPI susWriteConsoleA(
	_In_ SUS_FILE hConsoleOutput,
	_In_ LPCSTR text,
	_In_ INT len)
{
	SUS_PRINTDL("Text output to the console");
	DWORD numberOfCharsWritten;
	if (!WriteConsoleA(hConsoleOutput, text, len, &numberOfCharsWritten, NULL)) {
		SUS_PRINTDE("Couldn't output text to the console");
		SUS_PRINTDC(GetLastError());
		return 0;
	}
	return numberOfCharsWritten;
}
// Write text to the console
INT SUSAPI susWriteConsoleW(
	_In_ SUS_FILE hConsoleOutput,
	_In_ LPCWSTR text,
	_In_ INT len)
{
	SUS_PRINTDL("Text output to the console");
	DWORD numberOfCharsWritten;
	if (!WriteConsoleW(hConsoleOutput, text, len, &numberOfCharsWritten, NULL)) {
		SUS_PRINTDE("Couldn't output text to the console");
		SUS_PRINTDC(GetLastError());
		return 0;
	}
	return numberOfCharsWritten;
}
// Read the text from the console
INT SUSAPI susReadConsoleA(
	_In_ SUS_FILE hConsoleInput,
	_Out_writes_bytes_all_(len + 1) LPSTR buff,
	_In_ INT len)
{
	SUS_PRINTDL("Reading data from the console");
	DWORD numberOfCharsRead;
	if (!ReadConsoleA(hConsoleInput, buff, len, &numberOfCharsRead, NULL)) {
		*buff = '\0';
		SUS_PRINTDE("Couldn't read data from the console");
		SUS_PRINTDC(GetLastError());
		return 0;
	}
	buff[numberOfCharsRead] = '\0';
	SUS_PRINTDL("The data from the console has been read!");
	return numberOfCharsRead;
}
// Read the text from the console
INT SUSAPI susReadConsoleW(
	_In_ SUS_FILE hConsoleInput,
	_Out_writes_bytes_all_(len + 1) LPWSTR buff,
	_In_ INT len)
{
	SUS_PRINTDL("Reading data from the console");
	DWORD numberOfCharsRead;
	if (!ReadConsoleW(hConsoleInput, buff, len, &numberOfCharsRead, NULL)) {
		*buff = L'\0';
		SUS_PRINTDE("Couldn't read data from the console");
		SUS_PRINTDC(GetLastError());
		return 0;
	}
	buff[numberOfCharsRead] = L'\0';
	SUS_PRINTDL("The data from the console has been read!");
	return numberOfCharsRead;
}
