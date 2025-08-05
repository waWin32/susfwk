// networkCore.h
//
#ifndef _SUS_NETWORK_CORE_
#define _SUS_NETWORK_CORE_

#include <WinSock2.h>
#include "core.h"
#include "memory.h"

#define SUS_SOCKET_API_VERSION MAKEWORD(2, 2)

// socket structure
typedef struct sus_socket {
	SOCKET sock;
	SOCKADDR_IN addr;
	SIZE_T sizeAddr;
} SUS_SOCKET, *SUS_PSOCKET, *SUS_LPSOCKET;

// Initializing the winsock
DWORD SUSAPI susNetworkSetup();
// Network closure
VOID SUSAPI susNetworkCleanup();

// Configuring the socket
SUS_SOCKET SUSAPI susSocketSetup(_In_ ULONG addr, _In_ USHORT port);
// Build a socket
BOOL SUSAPI susBuildSocket(_Inout_ SUS_LPSOCKET lpSock);
// Closing a network socket
BOOL SUSAPI susCloseSocket(_In_ SUS_SOCKET sock);
// Binding an address to a socket
BOOL SUSAPI susBindSocket(_Inout_ SUS_LPSOCKET lpSock);
// Install Socket listening
BOOL SUSAPI susListenSocket(_Inout_ SUS_LPSOCKET lpSock);
// Connects to the server
BOOL SUSAPI susConnectToServer(_In_ SUS_SOCKET sock);

#endif /* !_SUS_NETWORK_CORE_ */
