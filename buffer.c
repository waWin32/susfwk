// buffer.c
//
#include "coreframe.h"
#include "include/susfwk/core.h"
#include "include/susfwk/buffer.h"

// ---------------------------------------------------------------------------------------

// Ñreate a new buffer
SUS_BUFFER SUSAPI susNewBuffer(_In_opt_ sus_size32_t capacity) {
	SUS_PRINTDL("Creating a new buffer");
	if (!capacity) capacity = SUS_BUFFER_CAPACITY;
	SUS_BUFFER buffer = sus_malloc(sizeof(SUS_BUFFER_STRUCT) + capacity);
	if (!buffer) return NULL;
	buffer->capacity = capacity;
	buffer->size = 0;
	return buffer;
}
// Delete Buffer
VOID SUSAPI susBufferDestroy(_In_ SUS_BUFFER buffer) {
	SUS_PRINTDL("Deleting a buffer");
	SUS_ASSERT(buffer);
	sus_free(buffer);
}
// Apply changes to the buffer
BOOL SUSAPI susBufferFlush(_Inout_ SUS_LPBUFFER lpBuffer) {
	SUS_ASSERT(lpBuffer && *lpBuffer);
	SUS_BUFFER newBuffer = (SUS_BUFFER)sus_realloc(*lpBuffer, sizeof(SUS_BUFFER_STRUCT) + (*lpBuffer)->capacity);
	if (!newBuffer) return FALSE;
	*lpBuffer = newBuffer;
	return TRUE;
}

// ---------------------------------------------------------------------------------------

// Guaranteed buffer size
static BOOL SUSAPI susBufferReserve(_Inout_ SUS_LPBUFFER lpBuffer, _In_ sus_size32_t reserve) {
	SUS_ASSERT(lpBuffer && *lpBuffer);
	SUS_BUFFER buffer = *lpBuffer;
	if (buffer->capacity < buffer->size + reserve) {
		buffer->capacity = (sus_size32_t)(((sus_float_t)buffer->size + reserve) * SUS_BUFFER_GROW_FACTOR);
		return susBufferFlush(lpBuffer);
	}
	return TRUE;
}
// Shrink the buffer to the minimum size
static BOOL SUSAPI susBufferCompress(_Inout_ SUS_LPBUFFER lpBuffer) {
	SUS_ASSERT(lpBuffer && *lpBuffer);
	SUS_BUFFER buffer = *lpBuffer;
	if (buffer->capacity > SUS_BUFFER_CAPACITY * SUS_BUFFER_GROW_FACTOR && buffer->capacity > buffer->size * SUS_BUFFER_SHRINK_THRESHOLD) {
		buffer->capacity = (sus_size32_t)((sus_float_t)buffer->size * SUS_BUFFER_GROW_FACTOR);
		return susBufferFlush(lpBuffer);
	}
	return TRUE;
}

// ---------------------------------------------------------------------------------------

// Paste the data into the buffer
SUS_LPMEMORY SUSAPI susBufferInsert(_Inout_ SUS_LPBUFFER lpBuffer, _In_ sus_size32_t pos, _In_opt_ sus_lpbyte_t data, _In_ sus_size32_t size)
{
	SUS_PRINTDL("Inserting an item into the buffer");
	SUS_ASSERT(lpBuffer && *lpBuffer && pos <= (*lpBuffer)->size);
	if (!susBufferReserve(lpBuffer, size)) return NULL;
	SUS_BUFFER buffer = *lpBuffer;
	sus_size32_t byteToMove = buffer->size - pos;
	if (byteToMove) sus_memmove(buffer->data + pos + size, buffer->data + pos, byteToMove);
	if (data) sus_memcpy(buffer->data + pos, data, size);
	buffer->size += size;
	return buffer->data + pos;
}
// Paste the data into the buffer
BOOL SUSAPI susBufferErase(_Inout_ SUS_LPBUFFER lpBuffer, _In_ sus_size32_t pos, _In_ sus_size32_t size)
{
	SUS_PRINTDL("Deleting an item from the buffer");
	SUS_ASSERT(lpBuffer && *lpBuffer && pos + size <= (*lpBuffer)->size);
	SUS_BUFFER buffer = *lpBuffer;
	sus_memmove(buffer->data + pos, buffer->data + pos + size, (sus_size_t)(buffer->size - pos));
	buffer->size -= size;
	return susBufferCompress(lpBuffer);
}
// Set the data to the buffer
SUS_LPMEMORY SUSAPI susBufferSet(_Inout_ SUS_LPBUFFER lpBuffer, _In_ sus_size32_t pos, _In_opt_ sus_lpbyte_t data, _In_ sus_size32_t size)
{
	SUS_ASSERT(lpBuffer && *lpBuffer && pos + size <= (*lpBuffer)->size);
	SUS_BUFFER buffer = *lpBuffer;
	if (data) sus_memcpy(buffer->data + pos, data, size);
	else sus_zeromem(buffer->data + pos, size);
	return buffer->data + pos;
}
// Swap two buffer elements
BOOL SUSAPI susBufferSwap(_Inout_ SUS_BUFFER buffer, _In_ sus_size32_t from, _In_ sus_size32_t to, _In_ sus_size32_t size)
{
	SUS_PRINTDL("Swapping buffer data");
	SUS_ASSERT(buffer && from + size <= buffer->size && to + size <= buffer->size);
	sus_byte_t sbuffer[SUS_BUFFER_TRANSFER_SIZE] = { 0 };
	sus_lpbyte_t buff = sbuffer;
	if (size > SUS_BUFFER_TRANSFER_SIZE) buff = sus_malloc(size);
	if (!buff) return FALSE;
	sus_memcpy(buff, buffer->data + from, size);
	sus_memcpy(buffer->data + from, buffer->data + to, size);
	sus_memcpy(buffer->data + to, buff, size);
	if (size > SUS_BUFFER_TRANSFER_SIZE) sus_free(buff);
	return TRUE;
}

// ---------------------------------------------------------------------------------------
