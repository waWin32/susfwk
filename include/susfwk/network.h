// networkCore.h
//
#ifndef _SUS_NETWORK_CORE_
#define _SUS_NETWORK_CORE_

#include <WinSock2.h>

#pragma comment(lib, "ws2_32.lib")
#pragma warning(disable: 4996)

// -------------------------------------------------------------------------------------------------------------

#define SUS_SOCKET_API_VERSION MAKEWORD(2, 2)
#define SUS_SOCKET_INIT_READ_BUFFER_SIZE 1024
#define SUS_SOCKET_INIT_WRITE_BUFFER_SIZE 1024
#define SUS_SOCKET_POLL_TIMEOUT 100
#define SUS_SOCKET_CHUNK_BUFFER_SIZE 512

// -------------------------------------------------------------------------------------------------------------

// Socket Messages
typedef enum sus_socket_message {
	SUS_SM_CREATE = 1,		// Called after creating a socket
	SUS_SM_START,			// Called after connecting to the host
	SUS_SM_DATA,			// Called when data is received
	SUS_SM_ERROR,			// Called when an error occurs
	SUS_SM_CLOSE,			// Called when the connection is closed
	SUS_SM_END			// Called after the connection is closed
} SUS_SOCKET_MESSAGE;
// Socket processing function
typedef VOID(SUSAPI* SUS_SOCKET_HANDLER)(SUS_OBJECT sock, SUS_SOCKET_MESSAGE uMsg, WPARAM wParam, LPARAM lParam);
// socket structure
typedef struct sus_socket {
	SOCKET				sock;		// System Socket Descriptor
	SUS_SOCKET_HANDLER	handler;	// Socket handler function
	SUS_BUFFER			readBuffer;	// Dynamic buffer for reading
	SUS_BUFFER			writeBuffer;// Dynamic buffer for writing
	SUS_OBJECT			userdata;	// User data
} SUS_SOCKET_STRUCT, *SUS_LPSOCKET_STRUCT, *SUS_SOCKET;
// The structure of the server socket
typedef struct sus_server_socket {
	SUS_SOCKET_STRUCT _PARENT_;
	SUS_VECTOR clients;					// SUS_SOCKET_STRUCT
	SUS_VECTOR clientfds;				// WSAPOLLFD
	SUS_SOCKET_HANDLER	heirHandler;	// Default client sockets callback
} SUS_SERVER_SOCKET_STRUCT, *SUS_LPSERVER_SOCKET_STRUCT, *SUS_SERVER_SOCKET;

// -------------------------------------------------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////////////////////////////////
//									Deinitializing the library										//
//////////////////////////////////////////////////////////////////////////////////////////////////////

// Initializing the winsock
DWORD SUSAPI susNetworkSetup();
// Network closure
VOID SUSAPI susNetworkCleanup();

//////////////////////////////////////////////////////////////////////////////////////////////////////
//									Basic socket operations											//
//////////////////////////////////////////////////////////////////////////////////////////////////////

// Configure the base socket
SUS_SOCKET_STRUCT SUSAPI susSocketSetup(
	_In_opt_ SUS_SOCKET_HANDLER handler,
	_In_opt_ SUS_OBJECT userData
);
// Build a socket
BOOL SUSAPI susBuildSocket(
	_In_ SUS_SOCKET sock
);
// Closing a network socket
BOOL SUSAPI susSocketClose(
	_In_ SUS_SOCKET sock
);
// Standard connection closure
BOOL SUSAPI susSocketShutdown(
	_In_ SUS_SOCKET sock
);
// Clean the socket
VOID SUSAPI susSocketCleanup(
	_Inout_ SUS_SOCKET sock
);

//////////////////////////////////////////////////////////////////////////////////////////////////////
//									Low-level operations											//
//////////////////////////////////////////////////////////////////////////////////////////////////////

// Automatically create an address for the socket
SUS_INLINE SOCKADDR_IN susSocketAddress(_In_ LPCSTR addr, _In_ USHORT port) {
	return (SOCKADDR_IN) {
		.sin_addr.s_addr = inet_addr(addr),
		.sin_family = AF_INET,
		.sin_port = htons(port)
	};
}
// Binding an address to a socket
BOOL SUSAPI susSocketBind(
	_In_ SUS_SOCKET sock,
	_In_ USHORT port
);
// Install Socket listening
BOOL SUSAPI susSocketListen(
	_In_ SUS_SOCKET sock
);
// Accept connection
BOOL SUSAPI susSocketAccept(
	_In_ SUS_SERVER_SOCKET server,
	_Out_ SUS_SOCKET client,
	_Out_opt_ LPSOCKADDR_IN pAddr
);
// Connects to the server
BOOL SUSAPI susSocketConnect(
	_In_ SUS_SOCKET sock, 
	_In_ LPCSTR addr,
	_In_ USHORT port
);

//////////////////////////////////////////////////////////////////////////////////////////////////////
//											I/O operations											//
//////////////////////////////////////////////////////////////////////////////////////////////////////

// Get data from a socket
BOOL SUSAPI susSocketRead(
	_Inout_ SUS_SOCKET sock
);
// Flushing the send buffer
SIZE_T SUSAPI susSocketFlush(
	_Inout_ SUS_SOCKET sock
);
// Check the availability of data to send
SUS_INLINE INT SUSAPI susSocketHasSendData(_Inout_ SUS_SOCKET sock) {
	return (INT)susBufferSize(sock->writeBuffer);
}
// Send data to the socket
SUS_INLINE VOID SUSAPI susSocketWrite(_Inout_ SUS_SOCKET sock, LPBYTE data, SIZE_T size) {
	SUS_ASSERT(data && size);
	susBufferAppend(&sock->writeBuffer, data, size);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
//									Fine-tuning the socket											//
//////////////////////////////////////////////////////////////////////////////////////////////////////

// Set the socket as non-blocking
SUS_INLINE BOOL SUSAPI susSocketSetNonBlocking(_In_ SUS_SOCKET sock, BOOL enabled) {
	SUS_ASSERT(sock && sock->sock != INVALID_SOCKET);
	return !ioctlsocket(sock->sock, FIONBIO, &(u_long)enabled);
}
// Set the timeout for the socket
SUS_INLINE BOOL SUSAPI susSocketSetTimeout(_In_ SUS_SOCKET sock, DWORD timeout) {
	SUS_ASSERT(sock && sock->sock != INVALID_SOCKET);
	return !(setsockopt(sock->sock, SOL_SOCKET, SO_SNDTIMEO, (PCHAR)&timeout, sizeof(timeout)) || setsockopt(sock->sock, SOL_SOCKET, SO_RCVTIMEO, (PCHAR)&timeout, sizeof(timeout)));
}
// configuring socket performance
SUS_INLINE BOOL SUSAPI susSocketTunePerformance(_In_ SUS_SOCKET sock) {
	int buffSize = 1024 * 1024;
	setsockopt(sock->sock, SOL_SOCKET, SO_RCVBUF, (PCHAR)&buffSize, sizeof(buffSize));
	setsockopt(sock->sock, SOL_SOCKET, SO_SNDBUF, (PCHAR)&buffSize, sizeof(buffSize));
	int nodelay = 1;
	return setsockopt(sock->sock, IPPROTO_TCP, TCP_NODELAY, (PCHAR)&nodelay, sizeof(nodelay));
}
// Get Socket user data
SUS_INLINE SUS_OBJECT SUSAPI susSocketGetUserData(_In_ SUS_SOCKET sock) {
	SUS_ASSERT(sock);
	return sock->userdata;
}
// Install the socket handler
SUS_INLINE VOID SUSAPI susSocketSetHandler(_In_ SUS_SOCKET sock, _In_ SUS_SOCKET_HANDLER handler) {
	SUS_ASSERT(sock);
	sock->handler = handler;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
//										Working with the server										//
//////////////////////////////////////////////////////////////////////////////////////////////////////

// Create a server
SUS_SERVER_SOCKET_STRUCT SUSAPI susServerSetup(
	_In_opt_ SUS_SOCKET_HANDLER handler,
	_In_opt_ SUS_SOCKET_HANDLER heirHandler,
	_In_opt_ SUS_OBJECT userData
);
// Set the socket as a listening server
BOOL SUSAPI susServerListen(
	_Inout_ SUS_SERVER_SOCKET server,
	_In_ USHORT port
);
// Accepting the client to the server
SUS_SOCKET SUSAPI susServerAccept(
	_In_ SUS_SERVER_SOCKET server
);
// Cleaning up server resources
VOID SUSAPI susServerCleanup(
	_Inout_ SUS_SERVER_SOCKET server
);

//////////////////////////////////////////////////////////////////////////////////////////////////////
//										Socket Managers												//
//////////////////////////////////////////////////////////////////////////////////////////////////////

// Process the socket
BOOL SUSAPI susSocketUpdate(
	_In_ SUS_SOCKET sock
);
// Update the server status
BOOL SUSAPI susServerUpdate(
	_In_ SUS_SERVER_SOCKET server
);

#endif /* !_SUS_NETWORK_CORE_ */
