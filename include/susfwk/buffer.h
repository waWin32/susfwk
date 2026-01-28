// buffer.h
//
#ifndef _SUS_BUFFER_
#define _SUS_BUFFER_

#pragma warning(push)
#pragma warning(disable: 4029)
#pragma warning(disable: 4200)

// ---------------------------------------------------------------------------------------

// Default base buffer size
#define SUS_BUFFER_CAPACITY 64
// Static size for transfer buffer
#define SUS_BUFFER_TRANSFER_SIZE 128
// Buffer growth rate
#define	SUS_BUFFER_GROW_FACTOR 1.6f
// The degree to which the buffer will be folded
#define	SUS_BUFFER_SHRINK_THRESHOLD 4.0f
// Dynamically expandable buffer
typedef struct sus_buffer {
	sus_size32_t	size;		// Occupied size in bytes
	sus_size32_t	capacity;	// Buffer capacity in bytes
	sus_byte_t		data[];		// Buffer Data
} SUS_BUFFER_STRUCT, *SUS_BUFFER, **SUS_LPBUFFER;

// ---------------------------------------------------------------------------------------

// Ñreate a new buffer
SUS_BUFFER SUSAPI susNewBuffer(_In_opt_ sus_size32_t capacity);
// Delete Buffer
VOID SUSAPI susBufferDestroy(_In_ SUS_BUFFER buffer);
// Apply changes to the buffer
BOOL SUSAPI susBufferFlush(_Inout_ SUS_LPBUFFER lpBuffer);

// ---------------------------------------------------------------------------------------

// Paste the data into the buffer
SUS_LPMEMORY SUSAPI susBufferInsert(_Inout_ SUS_LPBUFFER lpBuffer, _In_ sus_size32_t pos, _In_opt_ sus_lpbyte_t data, _In_ sus_size32_t size);
// Paste the data into the buffer
BOOL SUSAPI susBufferErase(_Inout_ SUS_LPBUFFER lpBuffer, _In_ sus_size32_t pos, _In_ sus_size32_t size);
// Set the data to the buffer
SUS_LPMEMORY SUSAPI susBufferSet(_Inout_ SUS_LPBUFFER lpBuffer, _In_ sus_size32_t pos, _In_opt_ sus_lpbyte_t data, _In_ sus_size32_t size);
// Swap two buffer elements
BOOL SUSAPI susBufferSwap(_Inout_ SUS_BUFFER buffer, _In_ sus_size32_t from, _In_ sus_size32_t to, _In_ sus_size32_t size);

// ---------------------------------------------------------------------------------------

// Insert the last element
SUS_INLINE SUS_LPMEMORY SUSAPI susBufferPush(_Inout_ SUS_LPBUFFER lpBuffer, _In_opt_ sus_lpbyte_t data, _In_ sus_size32_t size) { return susBufferInsert(lpBuffer, (*lpBuffer)->size, data, size); }
// Delete the last buffer element
SUS_INLINE BOOL SUSAPI susBufferPop(_Inout_ SUS_LPBUFFER lpBuffer, _In_ sus_size32_t size) { return susBufferErase(lpBuffer, (*lpBuffer)->size - size, size); }

// Insert the first element
SUS_INLINE SUS_LPMEMORY SUSAPI susBufferUnshift(_Inout_ SUS_LPBUFFER lpBuffer, _In_opt_ sus_lpbyte_t data, _In_ sus_size32_t size) { return susBufferInsert(lpBuffer, 0, data, size); }
// Delete the first buffer element
SUS_INLINE BOOL SUSAPI susBufferShift(_Inout_ SUS_LPBUFFER lpBuffer, _In_ sus_size32_t size) { return susBufferErase(lpBuffer, 0, size); }

// ---------------------------------------------------------------------------------------

#pragma warning(pop)

#endif // !_SUS_BUFFER_
