# SUSFramework2 (susfwk)
**A lightweight C Framework for creating applications with minimal dependencies.**

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![Windows](https://img.shields.io/badge/Platform-Windows-0078d7.svg)](https://learn.microsoft.com/en-us/windows/embedded/)

> Provides application performance, provides a convenient api and data structures. Reduces development time without losing performance.

The framework provides low-level access to system functions, adding debug information and error handling.
It allows you to customize the output of debug information (to the console or to a log file),
and it includes its own IO that provides input and output to different descriptors.
It provides an abstraction for creating and managing windows, and it includes a tool for creating widgets.

The framework presents its own style for writing safe and efficient code in C\C++ languages.
susfwk works without CRT, and a project written in it will only depend on system libraries.
This framework provides an interface that is similar to standard libraries, but has more functionality and is not dependent on them.

## Key Features ✨
- 🚫 **No CRT dependency** - Reduced footprint and increased portability
- 🛡️ **Enhanced security** - Built-in safe memory and resource management
- 📝 **Unified debugging** - Configurable output (console, file, or system debugger)
- 🧩 **Modular architecture** - Use only required components
- ✨ **CRT-like API** - Familiar interface without external dependencies

## How to install 🛠

1. Install Visual Studio
2. Create a project using the susfwk template
3. Install templates - place all project templates in the folder - 
C:\Users\Your user\Documents\Visual Studio 2022\Templates\ProjectTemplates
4. Go to the project properties and set the additional include directories - 
Properties -> C/C++ -> General -> Additional Include Directories - 
Specify the path to the 'include' folder here
5. In the properties, set additional library directories - 
Properties -> Linker -> General -> Additional Library Directories - 
Specify the path to the x64/Release folder here

## Quick start 🚀
```C
// main.c
//
#include "susfwk.h"
#pragma comment(lib, "susfwk.lib")

int _fltused = 1; // Required! (Due to the absence of CRT)

int main()
{
    SUS_CONSOLE_DEBUGGING(); // Creating a debugging console

    sus_printf("hello world!"); // Text output to the console

    SUS_FILE hFile = sus_fcreate("test.txt", FILE_ATTRIBUTE_NORMAL); // Creating a file test.txt
    sus_fprintf(hFile, "Formatted text - Test %d", 123); // writing format text to a file
    sus_fclose(hFile); // Closing the file

    ExitProcess(0);
}

```

## License ⚖
This project is distributed under the **MIT** license. 
Full text: [LICENSE](LICENSE.txt)

---
> **Support**: Please send your questions to [Discussions](https://github.com/waWin32/susfwk/discussions) 
> **Status**: In active development (stable version v1.0.0)
