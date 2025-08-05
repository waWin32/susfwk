// network.c
//
#include "coreframe.h"
#include "include/susfwk/core.h"
#include "include/susfwk/memory.h"
#include "include/susfwk/network.h"

DWORD SUSAPI susNetworkSetup()
{
	SUS_PRINTDL("Initializing the Winsock");
	WSADATA wsaData = { 0 };
	if (WSAStartup(SUS_SOCKET_API_VERSION, &wsaData)) {
		SUS_PRINTDE("Failed to initialize winsock");
		SUS_PRINTDC(WSAGetLastError());
		return WSAGetLastError();
	}
	SUS_PRINTDL("Winsock has been successfully initialized!");
	return 0;
}
// Network closure
VOID SUSAPI susNetworkCleanup()
{
	SUS_PRINTDL("Network closure");
	if (WSACleanup()) {
		SUS_PRINTDE("Couldn't clean the network");
		SUS_PRINTDC(WSAGetLastError());
		return;
	}
	SUS_PRINTDL("The network has been successfully closed");
}

// Configuring the socket
SUS_SOCKET SUSAPI susSocketSetup(_In_ ULONG addr, _In_ USHORT port)
{
	SUS_PRINTDL("Configuring the socket");
	SUS_SOCKET sock = {
		.sock = 0,
		.sizeAddr = sizeof(SOCKADDR_IN),
		.addr = {
			.sin_addr.s_addr = htonl(addr),
			.sin_family = AF_INET,
			.sin_port = htons(port)
		}
	};
	SUS_PRINTDL("The socket has been successfully configured");
	return sock;
}
// Build a socket
BOOL SUSAPI susBuildSocket(_Inout_ SUS_LPSOCKET lpSock)
{
	SUS_PRINTDL("Configuring the socket");
	lpSock->sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (lpSock->sock == INVALID_SOCKET) {
		SUS_PRINTDE("Couldn't build socket");
		SUS_PRINTDC(WSAGetLastError());
		return FALSE;
	}
	SUS_PRINTDL("The socket has been successfully configured");
	return TRUE;
}
// Closing a network socket
BOOL SUSAPI susCloseSocket(_In_ SUS_SOCKET sock)
{
	SUS_PRINTDL("Closing a network socket");
	SUS_ASSERT(sock.sock);
	if (closesocket(sock.sock) == SOCKET_ERROR) {
		SUS_PRINTDE("Couldn't close network socket");
		SUS_PRINTDC(WSAGetLastError());
		return FALSE;
	}
	SUS_PRINTDL("The network socket has been successfully closed");
	return TRUE;
}
// Binding an address to a socket
BOOL SUSAPI susBindSocket(_Inout_ SUS_LPSOCKET lpSock)
{
	SUS_PRINTDL("Binding an address to a socket");
	SUS_ASSERT(lpSock && lpSock->sock);
	if (bind(lpSock->sock, (SOCKADDR*)&lpSock->addr, (int)lpSock->sizeAddr) == SOCKET_ERROR) {
		SUS_PRINTDE("Failed to bind the socket to an address");
		SUS_PRINTDC(WSAGetLastError());
		return FALSE;
	}
	SUS_PRINTDL("The socket has been successfully bound to the address");
	return TRUE;
}
// Install Socket listening
BOOL SUSAPI susListenSocket(_Inout_ SUS_LPSOCKET lpSock)
{
	SUS_PRINTDL("Installing Socket listening");
	SUS_ASSERT(lpSock && lpSock->sock);
	if (listen(lpSock->sock, SOMAXCONN) == SOCKET_ERROR) {
		SUS_PRINTDE("Socket listening could not be set");
		SUS_PRINTDC(WSAGetLastError());
		return FALSE;
	}
	SUS_PRINTDL("The server listens for incoming connections");
	return TRUE;
}
// Connects to the server
BOOL SUSAPI susConnectToServer(_In_ SUS_SOCKET sock)
{
	SUS_PRINTDL("Connecting to the server ...");
	SUS_ASSERT(sock.sock);
	if (connect(sock.sock, (SOCKADDR*)&sock.addr, (int)sock.sizeAddr) == SOCKET_ERROR) {
		SUS_PRINTDE("Couldn't connect to the server");
		SUS_PRINTDC(WSAGetLastError());
		return FALSE;
	}
	SUS_PRINTDL("Successful connection to the server");
	return TRUE;
}
