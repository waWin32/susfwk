// networkCore.h
//
#ifndef _SUS_NETWORK_CORE_
#define _SUS_NETWORK_CORE_

#include <WinSock2.h>
#include <WS2tcpip.h>
#include "core.h"
#include "memory.h"
#include "vector.h"
#include "linkedList.h"

#pragma comment(lib, "ws2_32.lib")
#pragma warning(disable: 4996)

#define SUS_SOCKET_API_VERSION MAKEWORD(2, 2)
#define SUS_SOCKET_POLL_TIMEOUT 1000

#ifndef SUS_SOCKET_READ_BUFFER_SIZE
#define SUS_SOCKET_READ_BUFFER_SIZE 1024
#endif // !SUS_SOCKET_READ_BUFFER_SIZE

#ifndef SUS_SOCKET_WRITE_BUFFER_SIZE
#define SUS_SOCKET_WRITE_BUFFER_SIZE 1024
#endif // !SUS_SOCKET_WRITE_BUFFER_SIZE

typedef enum sus_sock_message {
	SUS_SOCK_NCCREATE = 1,
	SUS_SOCK_CREATE,
	SUS_SOCK_DATA,
	SUS_SOCK_START,
	SUS_SOCK_ACCEPT,
	SUS_SOCK_ERROR,
	SUS_SOCK_TIMEOUT,
	SUS_SOCK_CLOSE,
	SUS_SOCK_END
} SUS_SOCK_MESSAGE;

// Socket processing function
typedef DWORD(SUSAPI* SUS_SOCKET_HANDLER)(SUS_OBJECT sock, UINT uMsg, LPARAM lParam, WPARAM wParam);

typedef enum sus_sock_flags {
	SUS_SOCKFLAG_AUTOMATIC_FLUSHING = 1,
	SUS_SOCKFLAG_IMMEDIATE_PROCESSING
} SUS_SOCK_FLAGS;

// socket structure
typedef struct sus_socket {
	SOCKET sock;
	SUS_SOCKET_HANDLER handler;
	SUS_BUFFER readBuffer;
	SUS_BUFFER writeBuffer;
	sus_flag flags;
} SUS_SOCKET, *SUS_LPSOCKET;

// Initializing the winsock
DWORD SUSAPI susNetworkSetup();
// Network closure
VOID SUSAPI susNetworkCleanup();

//
SUS_INLINE SUS_SOCKET SUSAPI susInitSocket(_In_ SUS_SOCKET_HANDLER handler) {
	return (SUS_SOCKET) {
		.sock = INVALID_SOCKET,
		.handler = handler,
		.readBuffer = susNewBuffer(SUS_SOCKET_READ_BUFFER_SIZE),
		.writeBuffer = susNewBuffer(SUS_SOCKET_WRITE_BUFFER_SIZE),
		.flags = 0,
	};
}
// Build a socket
SUS_SOCKET SUSAPI susBuildSocket(_In_ SUS_SOCKET_HANDLER handler, _In_opt_ LPVOID userData);
// Closing a network socket
BOOL SUSAPI susSocketClose(_In_ SUS_LPSOCKET sock);
// Binding an address to a socket
BOOL SUSAPI susSocketBind(_In_ SUS_SOCKET sock, _In_ USHORT port);
// Install Socket listening
BOOL SUSAPI susSocketListen(_In_ SUS_SOCKET sock);
// Accept connection
SUS_SOCKET SUSAPI susSocketAccept(_In_ SUS_SOCKET server, _In_ SUS_SOCKET_HANDLER handler, _Out_opt_ PSOCKADDR_IN addr, _In_opt_ LPVOID userData);
// Connects to the server
SOCKADDR_IN SUSAPI susConnectToServer(_In_ SUS_SOCKET sock, _In_ LPCSTR addr, _In_ USHORT port);
// Polling socket events
BOOL SUSAPI susSocketPollEvents(_Inout_ SUS_LPSOCKET sock);
// Clean the socket
SUS_INLINE VOID SUSAPI susSocketCleanup(_Inout_ SUS_LPSOCKET sock) {
	sock->sock = INVALID_SOCKET;
	susBufferDestroy(sock->readBuffer);
	susBufferDestroy(sock->writeBuffer);
}

// Set the possibility of reusing the address
SUS_INLINE BOOL SUSAPI susSetSocketReuseAddr(SOCKET sock, BOOL enable) {
	return setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (PCHAR)&enable, sizeof(enable)) == 0;
}
// Set the number of ip hops for a socket
SUS_INLINE BOOL SUSAPI susSetSocketTtl(SOCKET sock, DWORD ttl) {
	return setsockopt(sock, IPPROTO_TCP, IP_TTL, (PCHAR)&ttl, sizeof(ttl)) == 0;
}
// Set the socket address as exclusive
SUS_INLINE BOOL SUSAPI susSetSocketExclusiveAddrUse(SOCKET sock, BOOL enable) {
	return setsockopt(sock, SOL_SOCKET, SO_EXCLUSIVEADDRUSE, (PCHAR)&enable, sizeof(enable)) == 0;
}
// Disable the Neigle algorithm (Higher performance)
SUS_INLINE BOOL SUSAPI susSetSocketNoDelay(SOCKET sock, BOOL disableNagle) {
	return setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, (PCHAR)&disableNagle, sizeof(disableNagle)) == 0;
}
// Set the behavior of a socket when it is closed
SUS_INLINE BOOL SUSAPI susSetSocketLinger(SOCKET sock, BOOL enable, USHORT timeoutSec) {
	struct linger opt = { (USHORT)enable, timeoutSec };
	return setsockopt(sock, SOL_SOCKET, SO_LINGER, (PCHAR)&opt, sizeof(opt)) == 0;
}
// Set a timeout for receiving data
SUS_INLINE BOOL SUSAPI susSetSocketRecvTimeout(SOCKET sock, DWORD timeout) {
	return setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (PCHAR)&timeout, sizeof(timeout)) == 0;
}
// Set a timeout for sending data
SUS_INLINE BOOL SUSAPI susSetSocketSendTimeout(SOCKET sock, DWORD timeout) {
	return setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, (PCHAR)&timeout, sizeof(timeout)) == 0;
}
// Set a timeout for sending data
SUS_INLINE BOOL SUSAPI susSetSocketNonBlocking(SOCKET sock, BOOL mode) {
	return ioctlsocket(sock, FIONBIO, (u_long*)&mode) == 0;
}
// Set a timeout for sending and receiving data
SUS_INLINE BOOL SUSAPI susSetSocketTimeout(SOCKET sock, DWORD timeout) {
	if (!susSetSocketRecvTimeout(sock, timeout)) return FALSE;
	return susSetSocketSendTimeout(sock, timeout);
}
// Set a timeout for sending and receiving data
SUS_INLINE INT SUSAPI susGetSocketError(SOCKET sock) {
	INT errorCode = 0, errorCodeSize = sizeof(errorCode);
	getsockopt(sock, SOL_SOCKET, SO_ERROR, (PCHAR)&errorCode, &errorCodeSize);
	return errorCode;
}

// Create a server
SUS_SOCKET SUSAPI susCreateServer(SUS_SOCKET_HANDLER handler, _In_opt_ LPVOID userData);
// Start the server
BOOL SUSAPI susServerListen(_Inout_ SUS_SOCKET sock, _In_ USHORT port);

// Get data from a socket
SUS_INLINE BOOL SUSAPI susSocketRead(_Inout_ SUS_LPSOCKET sock) {
	BYTE chunkBuffer[SUS_SOCKET_READ_BUFFER_SIZE] = { 0 };
	INT bytesRead;
	do {
		bytesRead = recv(sock->sock, (PCHAR)chunkBuffer, (INT)sizeof(chunkBuffer), 0);
		if (!bytesRead) {
			sock->handler(&sock, SUS_SOCK_CLOSE, 0, 0);
			return FALSE;
		}
		if (bytesRead == SOCKET_ERROR) {
			INT err = WSAGetLastError();
			if (err == WSAEWOULDBLOCK) return TRUE;
			sock->handler(&sock, SUS_SOCK_ERROR, (LPARAM)err, 0);
			return FALSE;
		}
		if (sock->flags & SUS_SOCKFLAG_IMMEDIATE_PROCESSING) {
			sock->handler(&sock, SUS_SOCK_DATA, (LPARAM)sock->readBuffer->data, sock->readBuffer->size);
		}
		susBufferAppend(&sock->readBuffer, chunkBuffer, bytesRead);
	} while (bytesRead == sizeof(chunkBuffer));
	sock->handler(&sock, SUS_SOCK_DATA, (LPARAM)sock->readBuffer->data, sock->readBuffer->size);
	susBufferClear(sock->readBuffer);
	return TRUE;
}
// Flushing the send buffer
SUS_INLINE BOOL SUSAPI susSocketFlush(_Inout_ SUS_LPSOCKET sock) {
	while (sock->writeBuffer->size) {
		INT bytesWrite = send(sock->sock, (PCHAR)sock->writeBuffer->data, (INT)sock->writeBuffer->size, 0);
		if (bytesWrite == SOCKET_ERROR) {
			INT err = WSAGetLastError();
			if (err == WSAEWOULDBLOCK) return TRUE;
			sock->handler(&sock, SUS_SOCK_ERROR, (LPARAM)err, sock->writeBuffer->size);
			return FALSE;
		}
		susBufferErase(&sock->writeBuffer, 0, bytesWrite);
	};
	return TRUE;
}
// Send data to the socket
SUS_INLINE BOOL SUSAPI susSocketWrite(_Inout_ SUS_LPSOCKET sock, LPBYTE data, SIZE_T size) {
	susBufferAppend(&sock->writeBuffer, data, size);
	if (sock->flags & SUS_SOCKFLAG_AUTOMATIC_FLUSHING) return susSocketFlush(sock);
	return TRUE;
}

#endif /* !_SUS_NETWORK_CORE_ */
