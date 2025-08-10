// deftypes.h
//
#ifndef _SUS_DEFINE_TYPES_
#define _SUS_DEFINE_TYPES_

#ifdef _M_X64 
#define SUS_SYSTEM_BITDEPTH 64
#else
#define SUS_SYSTEM_BITDEPTH 32
#endif // !_M_X64 


// =========================================================
// Basic data types
// =========================================================

// Minimum

typedef char sus_i8;
typedef short sus_i16;
typedef int sus_i32;
typedef long long sus_i64;

typedef unsigned char sus_u8;
typedef unsigned short sus_u16;
typedef unsigned int sus_u32;
typedef unsigned long long sus_u64;

typedef float sus_f32;
typedef double sus_f64;
typedef long double sus_f80;

typedef void* sus_ptr;
#ifdef _WIN64
typedef sus_u64 sus_size_t;
#else
typedef sus_u32 sus_size_t;
#endif // !_WIN64

// Assembler data types :

typedef sus_u8 sus_byte;
typedef sus_u16 sus_word;
typedef sus_u32 sus_dword;
typedef sus_u64 sus_qword;
typedef sus_f32 sus_float;
typedef sus_f64 sus_double;
typedef sus_f80 sus_exdouble;

#ifndef TRUE
#define TRUE 1
#endif // !TRUE
#ifndef FALSE
#define FALSE 0
#endif // !FALSE
#ifndef NULL
#define NULL ((sus_ptr)0)
#endif // !NULL

// =========================================================
// Wrapper data types
// =========================================================

// Types for flags :

typedef sus_u8 SUS_FLAG8, SUS_BITMASK8;
typedef sus_u16 SUS_FLAG16, SUS_BITMASK16;
typedef sus_u32 SUS_FLAG32, SUS_BITMASK32;
typedef sus_u64 SUS_FLAG64, SUS_BITMASK64;
typedef SUS_BITMASK32 SUS_BITMASK;

// custom data types :

typedef sus_i32 SUS_STATUS;
typedef sus_i32 SUS_RESULT;

// =========================================================

#ifndef VOID
#define VOID void
#endif // !VOID
#ifndef CONST
#define CONST const
#endif // !CONST

#define MAXPTR sizeof(sus_ptr)
#define MAXSIZE MAXPTR

typedef HANDLE SUS_FILE;

#define sus_fclose(hFile)	CloseHandle(hFile)

#endif /* !_SUS_DEF_TYPES_ */
