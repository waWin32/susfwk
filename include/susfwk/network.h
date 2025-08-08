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

#pragma warning(disable: 4996)

#define SUS_SOCKET_API_VERSION MAKEWORD(2, 2)

typedef enum sus_sock_message {
	SUS_SOCK_NCCREATE = 1,
	SUS_SOCK_CREATE,
	SUS_SOCK_DATA,
	SUS_SOCK_START,
	SUS_SOCK_ACCEPT,
	SUS_SOCK_ERROR,
	SUS_SOCK_CLOSE,
	SUS_SOCK_END
} SUS_SOCK_MESSAGE;

// Socket processing function
typedef VOID(SUSAPI* SUSNET_HANDLER)(SOCKET sock, UINT uMsg, LPARAM lParam, WPARAM wParam);

// socket structure
typedef struct sus_socket_context {
	SOCKET sock;
	SUSNET_HANDLER handler;
} SUS_SOCKET_CONTEXT, *SUS_PSOCKET_CONTEXT, *SUS_LPSOCKET_CONTEXT;

// Initializing the winsock
DWORD SUSAPI susNetworkSetup();
// Network closure
VOID SUSAPI susNetworkCleanup();

// Create a new socket context
SUS_INLINE SUS_SOCKET_CONTEXT SUSAPI susCreateSocket(_In_ SUSNET_HANDLER handler) {
	return (SUS_SOCKET_CONTEXT) { .handler = handler, .sock = INVALID_SOCKET };
}
// Build a socket
BOOL SUSAPI susBuildSocket(_Inout_ SUS_LPSOCKET_CONTEXT sct);
// Closing a network socket
BOOL SUSAPI susCloseSocket(_In_ SUS_SOCKET_CONTEXT sct);
// Binding an address to a socket
BOOL SUSAPI susBindSocket(_Inout_ SUS_SOCKET_CONTEXT sct, _In_ USHORT port);
// Install Socket listening
BOOL SUSAPI susListenSocket(_Inout_ SUS_SOCKET_CONTEXT sct);
// Connects to the server
SOCKADDR_IN SUSAPI susConnectToServer(_In_ SUS_SOCKET_CONTEXT sct, _In_ LPCSTR addr, _In_ USHORT port);

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
// Set a timeout for sending and receiving data
SUS_INLINE BOOL SUSAPI susSetSocketTimeout(SOCKET sock, DWORD timeout) {
	if (!susSetSocketRecvTimeout(sock, timeout)) return FALSE;
	return susSetSocketSendTimeout(sock, timeout);
}

// Create a server
SUS_SOCKET_CONTEXT SUSAPI susCreateServer(SUSNET_HANDLER handler);
// Start the server
BOOL SUSAPI susServerListen(_Inout_ SUS_SOCKET_CONTEXT sct, _In_ USHORT port);

#endif /* !_SUS_NETWORK_CORE_ */
