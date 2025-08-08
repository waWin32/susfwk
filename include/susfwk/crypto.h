// crypto.h
//
#ifndef _SUS_CRYPTO_
#define _SUS_CRYPTO_

#include <wincrypt.h>

// Encryption key
typedef struct sus_encryption_key {
	LPBYTE key;
	SIZE_T keySize;
} SUS_ENCRYPTION_KEY, *SUS_PENCRYPTION_KEY, *SUS_LPENCRYPTION_KEY;

// Creating an encryption key
SUS_ENCRYPTION_KEY SUSAPI susCreateEncryptionKey(
	_In_ CONST BYTE* key,
	_In_ SIZE_T size
);
// Creating an encryption key
BOOL SUSAPI susSetEncryptionKey(
	_Inout_ SUS_ENCRYPTION_KEY* ekey,
	_In_ CONST BYTE* key,
	_In_ SIZE_T size
);
// Generate a random key
SUS_ENCRYPTION_KEY SUSAPI susGenerateEncryptionKey(
	_In_ SIZE_T keySize
);
// Release the encryption key
VOID SUSAPI susDestroyEncryptionKey(
	_In_ SUS_ENCRYPTION_KEY key
);
// XOR string encryption
VOID susStringEncryption(
	_Inout_ LPBYTE data,
	_In_	SIZE_T size,
	_In_	SUS_ENCRYPTION_KEY key
);

// Creating a encryption key
#define susCreateKeyA(key) susCreateEncryptionKey(key, lstrlenA(key))
// Creating a encryption key
#define susCreateKeyW(key) susCreateEncryptionKey(key, lstrlenW(key))
// Setting the encryption key
#define susSetKeyA(ekey, key) susSetEncryptionKey(ekey, key, lstrlenA(key))
// Setting the encryption key
#define susSetKeyW(ekey, key) susSetEncryptionKey(ekey, key, lstrlenW(key))
// Release a encryption key
#define susDestroyKey(ekey) susDestroyEncryptionKey(ekey)

#ifdef UNICODE
#define susCreateKey susCreateKeyW
#define susSetKey susSetKeyW
#else
#define susCreateKey susCreateKeyA
#define susSetKey susSetKeyA
#endif // !UNICODE

// ---------------------------------------------------

// Hashing algorithm
typedef enum sus_hash_algorithm {
	SUS_HASH_MD5 = 16,		// 16 bytes
	SUS_HASH_SHA1 = 20,		// 20 bytes
	SUS_HASH_SHA256 = 32,	// 32 bytes
	SUS_HASH_SHA512 = 64	// 64 bytes
} SUS_HASH_ALGORITHM;

#endif /* !_SUS_CRYPTO_ */
