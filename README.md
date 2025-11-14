# SUSFramework2 (susfwk)
**A lightweight C Framework for creating applications with minimal dependencies.**

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![Windows](https://img.shields.io/badge/Platform-Windows-0078d7.svg)](https://learn.microsoft.com/en-us/windows/embedded/)
[![Status](https://img.shields.io/badge/Status-Stable%20v1.0.0-brightgreen.svg)](https://github.com/waWin32/susfwk/releases)

> Provides application performance, provides a convenient api and data structures. Reduces development time without losing performance.
## Overview

susfwk is a high-performance, high-level framework for C that combines extensive functionality with minimal footprint. It provides a comprehensive development ecosystem while maintaining zero dependencies on CRT or external libraries.

## Framework Capabilities

The framework delivers an expressive high-level API that simplifies complex tasks without compromising performance. It includes **extensive data structures** (including `SUS_HASHMAP`, `SUS_JSON`, `SUS_LIST` and dozens more), a **complete Entity-Component-System**, **abstractions for window management and widget creation**, and a **comprehensive networking stack** supporting multiple protocols across different layers, including the custom application-level JNET protocol. All components are backed by unified I/O systems and comprehensive debugging facilities.

## Key Features ✨

- **🚫 Zero Dependencies** - No CRT, no external libraries - pure system-level performance
- **⚡ High Performance** - Optimized execution with minimal overhead
- **📦 Extensive Standard Library** - Hundreds of modules covering all development needs
- **🔧 High-Level API** - Developer-friendly interfaces for rapid development
- **🛡️ Built-in Safety** - Comprehensive memory management and error handling
- **📝 Unified Debugging** - Configurable output (console, file, or system debugger)
- **🌐 Complete Networking** - Multi-protocol support from transport to application layers
- **🎮 Native ECS** - Full-featured Entity-Component-System for complex applications
- **🧩 Modular Architecture** - Use only the components you need

## Core Modules (Selected)

*This is just a small sample of available modules:*

| Category | Key Modules |
|----------|-------------|
| **Core Infrastructure** | `core.h`, `memory.h`, `debug.h`, `deftypes.h` |
| **Data Structures** | `vector.h`, `hashtable.h`, `linkedlist.h`, `bitset.h` |
| **I/O & Formatting** | `iostream.h`, `fileio.h`, `conio.h`, `format.h` |
| **Networking** | `network.h`, `jnet.h`, `httprequest.h` |
| **Application Development** | `windowapi.h`, `ecs.h`, `thrprocessapi.h` |
| **Utilities** | `string.h`, `math.h`, `crypto.h`, `json.h` |

*Plus many more specialized modules for graphics, system integration, and advanced algorithms.*

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
