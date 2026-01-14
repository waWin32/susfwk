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

typedef char sus_i8_t;
typedef short sus_i16_t;
typedef int sus_i32_t;
typedef long long sus_i64_t;

typedef unsigned char sus_u8_t, * sus_pu8_t;
typedef unsigned short sus_u16_t, *sus_pu16_t;
typedef unsigned int sus_u32_t, *sus_pu32_t;
typedef unsigned long long sus_u64_t;

typedef float sus_f32_t;
typedef double sus_f64_t;
typedef long double sus_f80_t;

typedef void* sus_ptr_t;
typedef void* sus_pvoid_t;
#ifdef _WIN64
typedef sus_u64_t sus_size_t;
#else
typedef sus_u32_t sus_size_t;
#endif // !_WIN64

typedef unsigned char sus_ubyte_t, *sus_lpubyte_t;
typedef char sus_byte_t, *sus_lpbyte_t;
typedef unsigned int sus_uint_t, * sus_puint_t;
typedef unsigned char sus_uint8_t, * sus_puint8_t;
typedef unsigned short sus_uint16_t, * sus_puint16_t;
typedef unsigned int sus_uint32_t, * sus_puint32_t;
typedef unsigned long long sus_uint64_t, * sus_puint64_t;
typedef int sus_int_t, *sus_pint_t;
typedef char sus_int8_t, * sus_pint8_t;
typedef short sus_int16_t, * sus_pint16_t;
typedef int sus_int32_t, * sus_pint32_t;
typedef long long sus_int64_t, * sus_pint64_t;
typedef float sus_float_t, *sus_pfloat_t;
typedef double sus_double_t, *sus_pdouble_t;

typedef char sus_char_t;
typedef unsigned short sus_wchar_t;

typedef _Null_terminated_ sus_char_t* sus_str_t;
typedef _Null_terminated_ const sus_char_t* sus_cstr_t;
typedef _Null_terminated_ sus_wchar_t* sus_wstr_t;
typedef _Null_terminated_ const sus_wchar_t* sus_cwstr_t;

// =========================================================
// Wrapper data types
// =========================================================

// custom data types :

typedef sus_u64_t	sus_uintptr_t;
typedef sus_i64_t	sus_intptr_t;
typedef sus_uint_t sus_bool_t, SUS_BOOL;
typedef sus_uint64_t sus_result_t, SUS_RESULT;
typedef sus_uint64_t sus_param_t, SUS_PARAM;
typedef sus_ptr_t sus_userdata_t, SUS_USERDATA;

// =========================================================

#define MAXPTR sizeof(sus_ptr_t)
#define MAXSIZE MAXPTR

#ifdef _WIN32
typedef HANDLE SUS_FILE;
#else
typedef int SUS_FILE;
#endif // !_WIN32

#define sus_fclose(hFile)	CloseHandle(hFile)

// =========================================================

#endif /* !_SUS_DEF_TYPES_ */
