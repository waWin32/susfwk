// conio.h
//
#ifndef _SUS_CON_IO_
#define _SUS_CON_IO_

// Set the console color
SUS_INLINE BOOL SUSAPI susSetConsoleColor(WORD bgColor, WORD textColor) {
	return SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), textColor | (bgColor << 4));
}
// Reset the console color
SUS_INLINE BOOL SUSAPI susResetConsoleColor() {
	return SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
}
// Set the console cursor pos
SUS_INLINE BOOL SUSAPI susSetConsoleCursorPos(WORD x, WORD y) {
	return SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), (COORD) { .X = x, .Y = y });
}
// Create a console
SUS_INLINE BOOL SUSAPI susAllocConsole() {
	return AllocConsole();
}

// Write text to the console
INT SUSAPI susWriteConsoleA(
	_In_ SUS_FILE hConsoleOutput,
	_In_ LPCSTR text,
	_In_ INT len
);
// Write text to the console
INT SUSAPI susWriteConsoleW(
	_In_ SUS_FILE hConsoleOutput,
	_In_ LPCWSTR text,
	_In_ INT len
);

#ifdef UNICODE
#define susWriteConsole susWriteConsoleW
#else
#define susWriteConsole susWriteConsoleA
#endif // !UNICODE

// Read the text from the console
INT SUSAPI susReadConsoleA(
	_In_ SUS_FILE hConsoleInput,
	_Out_writes_bytes_all_(len + 1) LPSTR buff,
	_In_ INT len
);
// Read the text from the console
INT SUSAPI susReadConsoleW(
	_In_ SUS_FILE hConsoleInput,
	_Out_writes_bytes_all_(len + 1) LPWSTR buff,
	_In_ INT len
);

#ifdef UNICODE
#define susReadConsole susReadConsoleW
#else
#define susReadConsole susReadConsoleA
#endif // !UNICODE

#endif /* !_SUS_CON_IO_ */
