// crypto.c
//
#include "coreframe.h"
#include "include/susfwk/core.h"
#include "include/susfwk/string.h"
#include "include/susfwk/memory.h"
#include "include/susfwk/crypto.h"

// Creating an encryption key
SUS_ENCRYPTION_KEY SUSAPI susCreateEncryptionKey(
	_In_ CONST BYTE* key,
	_In_ SIZE_T size)
{
	SUS_PRINTDL("Creating an encryption key");
	SUS_ASSERT(key && size > 0);
	SUS_ENCRYPTION_KEY ekey = {
		.key = sus_malloc(size),
		.keySize = size
	};
	if (!(ekey.key)) {
		SUS_PRINTDE("Couldn't create a encryption key");
		return (SUS_ENCRYPTION_KEY) { 0 };
	}
	sus_memcpy(ekey.key, (CONST LPVOID)key, size);
	return ekey;
}
// Generate a random key
SUS_ENCRYPTION_KEY SUSAPI susGenerateEncryptionKey(_In_ SIZE_T keySize)
{
	LPBYTE key = sus_malloc(keySize);
	if (!key) return (SUS_ENCRYPTION_KEY) { 0 };
	HCRYPTPROV hProv = 0;
	if (!CryptAcquireContextW(&hProv, NULL, NULL, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT | CRYPT_SILENT)) {
		SUS_PRINTDE("Failed to generate secure random key");
		SUS_PRINTDC(GetLastError());
		sus_free(key);
		return (SUS_ENCRYPTION_KEY) { 0 };
	}
	if (!CryptGenRandom(hProv, (DWORD)keySize, key)) {
		SUS_PRINTDE("Failed to generate secure random key");
		SUS_PRINTDC(GetLastError());
		CryptReleaseContext(hProv, 0);
		sus_free(key);
		return (SUS_ENCRYPTION_KEY) { 0 };
	}
	CryptReleaseContext(hProv, 0);
	SUS_ENCRYPTION_KEY ekey = susCreateEncryptionKey(key, keySize);
	sus_free(key);
	return ekey;
}
// Creating an encryption key
BOOL SUSAPI susSetEncryptionKey(
	_Inout_ SUS_ENCRYPTION_KEY* ekey,
	_In_ CONST BYTE* key,
	_In_ SIZE_T size)
{
	SUS_PRINTDL("Creating an encryption key");
	SUS_ASSERT(ekey && size > 0 && key);
	ekey->key = sus_realloc(ekey->key, size);
	if (!ekey->key) {
		SUS_PRINTDE("Couldn't set a encryption key");
		return FALSE;
	}
	sus_memcpy(ekey->key, (CONST LPVOID)key, size);
	return TRUE;
}
// Release the encryption key
VOID SUSAPI susDestroyEncryptionKey(_In_ SUS_ENCRYPTION_KEY key)
{
	SUS_PRINTDL("clearing the encryption key");
	SUS_ASSERT(key.key);
	sus_free(key.key);
}
// XOR string encryption
VOID SUSAPI susStringEncryption(
	_Inout_ LPBYTE data,
	_In_	SIZE_T size,
	_In_	SUS_ENCRYPTION_KEY key)
{
	for (SIZE_T i = 0; i < size; i++) {
		data[i] ^= key.key[i % key.keySize];
	}
}
