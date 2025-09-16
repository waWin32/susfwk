// networkCore.h
//
#ifndef _SUS_NETWORK_CORE_
#define _SUS_NETWORK_CORE_

#include <WinSock2.h>
#include <WS2tcpip.h>

#pragma comment(lib, "ws2_32.lib")
#pragma warning(disable: 4996)

#define SUS_SOCKET_API_VERSION MAKEWORD(2, 2)
#define SUS_SOCKET_POLL_TIMEOUT 1000
#define SUS_SOCKET_READ_BUFFER_SIZE 1024
#define SUS_SOCKET_WRITE_BUFFER_SIZE 1024

// Socket Messages
typedef enum sus_sock_message {
	SUS_SOCK_NCCREATE = 1,	// Called before the socket is created
	SUS_SOCK_CREATE,		// Called after creating a socket
	SUS_SOCK_START,			// Called after connecting to the host
	SUS_SOCK_DATA,			// Called when data is received
	SUS_SOCK_ERROR,			// Called when an error occurs
	SUS_SOCK_TIMEOUT,		// It is called when the waiting time is over
	SUS_SOCK_CLOSE,			// Called when the connection is closed
	SUS_SOCK_END			// Called after the connection is closed
} SUS_SOCK_MESSAGE;

// Socket processing function
typedef VOID(SUSAPI* SUS_SOCKET_HANDLER)(SUS_OBJECT sock, SUS_SOCK_MESSAGE uMsg, WPARAM wParam, LPARAM lParam);

// socket structure
typedef struct sus_socket {
	SOCKET				sock;		//	System Socket Descriptor
	SUS_SOCKET_HANDLER	handler;	//	Socket handler function
	SUS_BUFFER			readBuffer;	//	Dynamic buffer for reading
	SUS_BUFFER			writeBuffer;//	Dynamic buffer for writing
} SUS_SOCKET, *SUS_LPSOCKET;

// Initializing the winsock
DWORD SUSAPI susNetworkSetup();
// Network closure
VOID SUSAPI susNetworkCleanup();

// Get data from a socket
BOOL SUSAPI susSocketRead(_Inout_ SUS_LPSOCKET sock);
// Flushing the send buffer
BOOL SUSAPI susSocketFlush(_Inout_ SUS_LPSOCKET sock);

// Send data to the socket
SUS_INLINE BOOL SUSAPI susSocketWrite(_Inout_ SUS_LPSOCKET sock, LPBYTE data, SIZE_T size) {
	susBufferAppend(&sock->writeBuffer, data, size);
	return TRUE;
}

#endif /* !_SUS_NETWORK_CORE_ */
