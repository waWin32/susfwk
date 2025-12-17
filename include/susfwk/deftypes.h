// deftypes.h
//
#ifndef _SUS_DEFINE_TYPES_
#define _SUS_DEFINE_TYPES_

#if defined(_WIN64) || defined(__x86_64__) || defined(__ppc64__) || defined(__aarch64__)
#define _SUS_X64_
#else
#define _SUS_X86_
#endif // !x64

#ifdef _SUS_X64_ 
#define SUS_SYSTEM_BITDEPTH 64
#else
#define SUS_SYSTEM_BITDEPTH 32
#endif // !_SUS_X64_ 


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

typedef VOID* sus_ptr;
#ifdef _WIN64
typedef sus_u64 sus_size_t;
#else
typedef sus_u32 sus_size_t;
#endif // !_WIN64

typedef int sus_int, *sus_pint;
typedef unsigned int sus_uint, *sus_puint;
typedef sus_uint sus_uint32, *sus_puint32;
typedef long long sus_long, * sus_plong;
typedef unsigned long long sus_ulong, *sus_pulong;
typedef sus_ulong sus_uint64, *sus_puint64;
typedef float sus_float, *sus_pfloat;
typedef double sus_double, *sus_pdouble;

typedef char sus_char;
typedef unsigned short sus_wchar;

typedef _Null_terminated_ sus_char* sus_str;
typedef _Null_terminated_ const sus_char* sus_cstr;
typedef _Null_terminated_ sus_wchar* sus_wstr;
typedef _Null_terminated_ const sus_wchar* sus_cwstr;

// =========================================================
// Wrapper data types
// =========================================================

// Types for flags :

typedef sus_u8 sus_flag8;
typedef sus_u16 sus_flag16;
typedef sus_u32 sus_flag32;
typedef sus_u64 sus_flag64;
typedef sus_flag32 sus_flag;

// custom data types :

typedef sus_u32 sus_bool;
typedef sus_i32 sus_status;
typedef sus_i32 sus_result;

// =========================================================

#define MAXPTR sizeof(sus_ptr)
#define MAXSIZE MAXPTR

#ifdef _WIN32
typedef HANDLE SUS_FILE;
#else
typedef int SUS_FILE;
#endif // !_WIN32

#define sus_fclose(hFile)	CloseHandle(hFile)

// =========================================================

#endif /* !_SUS_DEF_TYPES_ */
