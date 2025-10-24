// crypto.h
//
#ifndef _SUS_CRYPTO_
#define _SUS_CRYPTO_

// Encryption key
typedef struct sus_encryption_key {
	LPBYTE key;
	SIZE_T keySize;
} SUS_ENCRYPTION_KEY, *SUS_LPENCRYPTION_KEY;

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

// ---------------------------------------------------

#endif /* !_SUS_CRYPTO_ */
