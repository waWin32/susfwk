# SUSFramework2
**A lightweight C framework for embedded systems that provides low-level access to the Windows system**

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![Windows](https://img.shields.io/badge/Platform-Windows-0078d7.svg)](https://learn.microsoft.com/en-us/windows/embedded/)

> Provides secure and simplified access to Windows system resources for embedded systems, reducing development time by 30-40%.

The framework provides low-level access to system functions, adding debug information and error handling.
It allows you to customize the output of debug information (to the console or to a log file),
and it includes its own IO that provides input and output to different descriptors.
It provides an abstraction for creating and managing windows, and it includes a tool for creating widgets.

The framework presents its own style for writing safe and efficient code in C\C++ languages.
susfwk works without CRT, and a project written in it will only depend on system libraries.
This framework provides an interface similar to standard libraries, but without relying on them.

## Key Features ✨
- 🚫 **No CRT dependency** - Reduced footprint and increased portability
- 🛡️ **Enhanced security** - Built-in safe memory and resource management
- 📝 **Unified debugging** - Configurable output (console, file, or system debugger)
- 🧩 **Modular architecture** - Use only required components
- ✨ **CRT-like API** - Familiar interface without external dependencies

## Quick start 🚀
```C
// WinMain.c
//
#include "susfwk.h"
#pragma comment(lib, "susf2.lib")

int _fltused = 1; // Required! (Due to the absence of CRT)

int WINAPI WinMain(
    _In_        HINSTANCE   hInstance,
    _In_opt_    HINSTANCE   hPrevInstance,
    _In_        LPSTR       lpCmdLine,
    _In_        int         nShowCmd)
{
    UNREFERENCED_PARAMETER(hInstance);
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);
    UNREFERENCED_PARAMETER(nShowCmd);

    SUS_CONSOLE_DEBUGGING(); // Creating a debugging console
    sus_printf("hello world!"); // Text output to the console

    SUS_FILE hFile = sus_fcreate("test.txt", FILE_ATTRIBUTE_NORMAL); // Creating a file test.txt
    sus_fwrite(hFile, "Text - hello world\n", -1); // writing to a text file
    sus_fprintf(hFile, "Formatted text - Test %d", 123); // writing format text to a file
    sus_fclose(hFile); // Closing the file

    Sleep(INFINITE);
    return 0;
}

```

## License ⚖
This project is distributed under the **MIT** license. 
Full text: [LICENSE](LICENSE.txt)

---
> **Support**: Please send your questions to [Discussions](https://github.com/waWin32/susfwk/discussions) 
> **Status**: In active development (stable version v1.0.0)
