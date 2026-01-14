// bitset.h
//
#ifndef _SUS_BITSET_
#define _SUS_BITSET_

#include "core.h"

// -----------------------------------------------

typedef sus_u8_t SUS_BITMASK8;
typedef sus_u16_t SUS_BITMASK16;
typedef sus_u32_t SUS_BITMASK32;
typedef sus_u64_t SUS_BITMASK64;
typedef SUS_BITMASK64 SUS_BITMASK;

// -----------------------------------------------

// Get a bitmask by shift
#define sus_bitmask(shift) (1ULL << (shift))
#define susBitmaskFor(start, i, mask, end) for (DWORD i = start; i < end; i++)
#define susBitmaskForeach(i, mask) susBitmaskFor(0, i, mask, sizeof(mask) * 8)

// Create a bitmask
SUS_INLINE SUS_BITMASK SUSAPI susBitmask64(UINT count, ...) {
	sus_va_list args;
	sus_va_start(args, count);
	SUS_BITMASK mask = 0;
	for (UINT i = 0; i < count; i++) {
		mask |= sus_bitmask(sus_va_arg(args, UINT));
	}
	sus_va_end(args);
	return mask;
}

#define susBitmask64op(a, op, b) (SUS_BITMASK) (a) op (b)
#define susBitmask64Cmp(a, b) (a) == (b)

// -----------------------------------------------

typedef struct {
	SUS_BITMASK64 low;
	SUS_BITMASK64 high;
} SUS_BITMASK128, *SUS_LPBITMASK128;

#define susBitmask128op(a, op, b) (SUS_BITMASK128) { .low = susBitmask64op((a).low, op, (b).low), .high = susBitmask64op((a).high, op, (b).high) }
#define susBitmask128Cmp(a, b) susBitmask64Cmp((a).low, (b).low) && susBitmask64Cmp((a).high, (b).high)
SUS_INLINE VOID SUSAPI susBitmask128Reset(SUS_LPBITMASK128 mask, UINT shift) {
	if (shift < 64) mask->low &= ~sus_bitmask(shift);
	else mask->high &= ~sus_bitmask(shift - 64);
}
SUS_INLINE VOID SUSAPI susBitmask128Set(SUS_LPBITMASK128 mask, UINT shift) {
	if (shift < 64) mask->low |= sus_bitmask(shift);
	else mask->high |= sus_bitmask(shift - 64);
}
SUS_INLINE BOOL SUSAPI susBitmask128Test(SUS_BITMASK128 mask, UINT shift) {
	if (shift < 64) return (mask.low & sus_bitmask(shift)) != 0;
	return (mask.high & sus_bitmask(shift - 64)) != 0;
}
SUS_INLINE BOOL SUSAPI susBitmask128Contains(SUS_BITMASK128 mask, SUS_BITMASK128 submask) {
	SUS_BITMASK128 contains = susBitmask128op(mask, &, submask);
	return susBitmask128Cmp(contains, submask);
}
// Create a bitmask
SUS_INLINE SUS_BITMASK128 SUSAPI susBitmask128(UINT count, ...) {
	sus_va_list args;
	sus_va_start(args, count);
	SUS_BITMASK128 mask = { 0 };
	for (UINT i = 0; i < count; i++) {
		susBitmask128Set(&mask, sus_va_arg(args, UINT));
	}
	sus_va_end(args);
	return mask;
}

// -----------------------------------------------

typedef struct {
	SUS_BITMASK128 low;
	SUS_BITMASK128 high;
} SUS_BITMASK256, *SUS_LPBITMASK256;

#define susBitmask256op(a, op, b) (SUS_BITMASK256) { .low = susBitmask128op((a).low, op, (b).low), .high = susBitmask128op((a).high, op, (b).high) }
#define susBitmask256Cmp(a, b) susBitmask128Cmp((a).low, (b).low) && susBitmask128Cmp((a).high, (b).high)
SUS_INLINE VOID SUSAPI susBitmask256Reset(SUS_LPBITMASK256 mask, UINT shift) {
	if (shift < 128) susBitmask128Reset(&mask->low, shift);
	else susBitmask128Reset(&mask->high, shift - 128);
}
SUS_INLINE VOID SUSAPI susBitmask256Set(SUS_LPBITMASK256 mask, UINT shift) {
	if (shift < 128) susBitmask128Set(&mask->low, shift);
	else susBitmask128Set(&mask->high, shift - 128);
}
SUS_INLINE BOOL SUSAPI susBitmask256Test(SUS_BITMASK256 mask, UINT shift) {
	if (shift < 128) return susBitmask128Test(mask.low, shift);
	return susBitmask128Test(mask.high, shift - 128);
}
SUS_INLINE BOOL SUSAPI susBitmask256Contains(SUS_BITMASK256 mask, SUS_BITMASK256 submask) {
	SUS_BITMASK256 contains = susBitmask256op(mask, &, submask);
	return susBitmask256Cmp(contains, submask);
}
// Create a bitmask
SUS_INLINE SUS_BITMASK256 SUSAPI susBitmask256(UINT count, ...) {
	sus_va_list args;
	sus_va_start(args, count);
	SUS_BITMASK256 mask = { 0 };
	for (UINT i = 0; i < count; i++) {
		susBitmask256Set(&mask, sus_va_arg(args, UINT));
	}
	sus_va_end(args);
	return mask;
}

// -----------------------------------------------

typedef struct {
	SUS_BITMASK256 low;
	SUS_BITMASK256 high;
} SUS_BITMASK512, *SUS_LPBITMASK512;

#define susBitmask512op(a, op, b) (SUS_BITMASK512) { .low = susBitmask256op((a).low, op, (b).low), .high = susBitmask256op((a).high, op, (b).high) }
#define susBitmask512Cmp(a, b) susBitmask256Cmp((a).low, (b).low) && susBitmask256Cmp((a).high, (b).high)
SUS_INLINE VOID SUSAPI susBitmask512Reset(SUS_LPBITMASK512 mask, UINT shift) {
	if (shift < 256) susBitmask256Reset(&mask->low, shift);
	else susBitmask256Reset(&mask->high, shift - 512);
}
SUS_INLINE VOID SUSAPI susBitmask512Set(SUS_LPBITMASK512 mask, UINT shift) {
	if (shift < 512) susBitmask256Set(&mask->low, shift);
	else susBitmask256Set(&mask->high, shift - 512);
}
SUS_INLINE BOOL SUSAPI susBitmask512Test(SUS_BITMASK512 mask, UINT shift) {
	if (shift < 512) return susBitmask256Test(mask.low, shift);
	return susBitmask256Test(mask.high, shift - 512);
}
SUS_INLINE BOOL SUSAPI susBitmask512Contains(SUS_BITMASK512 mask, SUS_BITMASK512 submask) {
	SUS_BITMASK512 contains = susBitmask512op(mask, &, submask);
	return susBitmask512Cmp(contains, submask);
}
// Create a bitmask
SUS_INLINE SUS_BITMASK512 SUSAPI susBitmask512(UINT count, ...) {
	sus_va_list args;
	sus_va_start(args, count);
	SUS_BITMASK512 mask = { 0 };
	for (UINT i = 0; i < count; i++) {
		susBitmask512Set(&mask, sus_va_arg(args, UINT));
	}
	sus_va_end(args);
	return mask;
}

// -----------------------------------------------

#endif /* !_SUS_BITSET_ */