/*++ BUILD Version: 0002    Increment this if a change has global effects

Copyright (c) Archon. All rights reserved.

Module Name:

	SUSCore.h

Abstract:

	The core of the utility kit library,
	which provides the core of the library

--*/

#ifndef _SUS_CORE_
#define _SUS_CORE_

/*The core of the sus library provides basic
* wrapper functions over system calls. It contains minimal
* functionality, but this module provides the necessary
* layer of abstraction over the system.*/

// Macros for the sus core //

/* A simple structure inheritance mechanism
In the structure, the inherited structure is marked as the parent,
and when used, access it via super. 
*/
#define _PARENT_ super
// Example of using PARENT:
/******************************************************************
	struct struct_base {
		int a;
		char name[64];
	};
	// Inherits the parent's structure - struct_base
	struct my_struct {
		struct struct_base _PARENT_; // There must be only one, and it must be at the beginning of the structure
		int b;
	};
	int my_func(struct my_struct s) {
		s.b = 5; // The Heir's field
		s.super.a = 20;			// Access to the parent's fields
		s.super.name = "petya";	// Access to the parent's fields
	}
******************************************************************/

// In your code, write 'int _fltused = 1;'
// extern int _fltused;

// -----------------------------------------------

#define SUSAPI __fastcall
#define SUSAPIV __cdecl
#define SUS_INLINE __inline
#define SUS_FORCEINLINE __forceinline
#define SUS_EXTERN extern
#define SUS_STATIC static
#define SUS_STRUCT struct

#ifdef _WIN32
	// Import a function from a library
	#define SUS_IMPORT __declspec(dllimport)
	// Mark a function as exported
	#define SUS_EXPORT __declspec(dllexport)
#else
	// Import a function from a library
	#define SUS_IMPORT __attribute__((visibility("default")))
	// Mark a function as exported
	#define SUS_EXPORT __attribute__((visibility("default")))
#endif // !_WIN32

// API for using the SUS library
#ifdef SUS_EXPORTS
#define SUSUSERAPI SUS_EXPORT
#else
#define SUSUSERAPI SUS_IMPORT
#endif // !DLL_EXPORTS

// -----------------------------------------------

// Element count in array
#define SUS_COUNT_OF(array) (sizeof(array) / sizeof(*(array)))
// The return value for exiting recursion
#define GETTING_OUT_RECURSION (DWORD)-1

#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)
#define WIDESTRINGIFY(x) L##x
#define WIDESTRING(x) WIDESTRINGIFY(x)


// Example of using SUS_TO_XARRAY:
/******************************************************************
	struct my_vector {
		float x;
		float y;
		float z;
	};
	float my_func(struct my_vector vec, int i) {
		return SUS_TO_FARRAY(vec)[i]; // Returns x if i == 0, y if i == 1, or z if i == 2.
	}
******************************************************************/

#ifdef _WIN32
#include <Windows.h>
#else
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#endif // !_WIN32

#include "deftypes.h"

#define SUS_OFFSET_OF(type, member)	((sus_size_t)&(((type*)0)->member))

// =======================================================================================//
//						API for an indefinite number of arguments						  //
// =======================================================================================//

// Alignment
#define SUS_ALIGN(size, align) (((size) + (align) - 1) & ~((align) - 1))

// Exit the program
#define sus_exit(code) ExitProcess(code)
#ifdef _DEBUG
// Emergency exit from the program
#define sus_error(code) FatalExit(code)
#else
// Emergency exit from the program
#define sus_error(code) ExitProcess(code)
#endif // !_DEBUG

#ifdef _WIN32
	typedef sus_u8_t* sus_va_list;
	#define sus_va_start(ap, last) (ap = (sus_va_list)&last + SUS_ALIGN(sizeof(last), sizeof(sus_size_t)))
	#define sus_va_arg(ap, type) (*(type*)((ap += SUS_ALIGN(sizeof(type), sizeof(sus_size_t))) - SUS_ALIGN(sizeof(type), sizeof(sus_size_t))))
	#define sus_va_end(ap) (ap = NULL)
#else
	typedef __builtin_va_list sus_va_list;
	#define sus_va_start(ap, last) __builtin_va_start(ap, last)
	#define sus_va_arg(ap, type)   __builtin_va_arg(ap, type)
	#define sus_va_end(ap)         __builtin_va_end(ap)
#endif // !_WIN32

#include "debug.h"
#include "error.h"
#include "string.h"
#include "memory.h"
#include "format.h"
#include "iostream.h"

#endif /* !_SUS_CORE_ */
