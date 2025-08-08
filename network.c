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

// Build a socket
BOOL SUSAPI susBuildSocket(_Inout_ SUS_LPSOCKET_CONTEXT sct)
{
	SUS_PRINTDL("Building a socket");
	SUS_ASSERT(sct);
	sct->handler(sct->sock, SUS_SOCK_NCCREATE, IPPROTO_TCP, AF_INET);
	sct->sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (sct->sock == INVALID_SOCKET) {
		sct->handler(sct->sock, SUS_SOCK_ERROR, WSAGetLastError(), SUS_SOCK_NCCREATE);
		SUS_PRINTDE("Couldn't build socket");
		SUS_PRINTDC(WSAGetLastError());
		return FALSE;
	}
	sct->handler(sct->sock, SUS_SOCK_CREATE, IPPROTO_TCP, AF_INET);
	SUS_PRINTDL("The socket has been successfully installed!");
	return TRUE;
}
// Closing a network socket
BOOL SUSAPI susCloseSocket(_In_ SUS_SOCKET_CONTEXT sct)
{
	SUS_PRINTDL("Closing a network socket");
	SUS_ASSERT(sct.sock);
	sct.handler(sct.sock, SUS_SOCK_CLOSE, 0, 0);
	if (closesocket(sct.sock) == SOCKET_ERROR) {
		sct.handler(sct.sock, SUS_SOCK_ERROR, WSAGetLastError(), SUS_SOCK_CLOSE);
		SUS_PRINTDE("Couldn't close network socket");
		SUS_PRINTDC(WSAGetLastError());
		return FALSE;
	}
	SUS_PRINTDL("The network socket has been successfully closed");
	return TRUE;
}
// Binding an address to a socket
BOOL SUSAPI susBindSocket(_Inout_ SUS_SOCKET_CONTEXT sct, _In_ USHORT port)
{
	SUS_PRINTDL("Binding an address to a socket");
	SUS_ASSERT(sct.sock);
	susSetSocketLinger(sct.sock, TRUE, 10);
	SOCKADDR_IN sockAddr = {
		.sin_addr.s_addr = INADDR_ANY,
		.sin_family = AF_INET,
		.sin_port = htons(port)
	};
	if (bind(sct.sock, (SOCKADDR*)&sockAddr, (int)sizeof(sockAddr)) == SOCKET_ERROR) {
		sct.handler(sct.sock, SUS_SOCK_ERROR, WSAGetLastError(), SUS_SOCK_CREATE);
		SUS_PRINTDE("Failed to bind the socket to an address");
		SUS_PRINTDC(WSAGetLastError());
		return FALSE;
	}
	SUS_PRINTDL("The socket has been successfully bound to the address");
	return TRUE;
}
// Install Socket listening
BOOL SUSAPI susListenSocket(_Inout_ SUS_SOCKET_CONTEXT sct)
{
	SUS_PRINTDL("Installing Socket listening");
	SUS_ASSERT(sct.sock);
	if (listen(sct.sock, SOMAXCONN) == SOCKET_ERROR) {
		sct.handler(sct.sock, SUS_SOCK_ERROR, WSAGetLastError(), SUS_SOCK_CREATE);
		SUS_PRINTDE("Socket listening could not be set");
		SUS_PRINTDC(WSAGetLastError());
		return FALSE;
	}
	SUS_PRINTDL("The server listens for incoming connections");
	return TRUE;
}
// Connects to the server
SOCKADDR_IN SUSAPI susConnectToServer(_In_ SUS_SOCKET_CONTEXT sct, _In_ LPCSTR addr, _In_ USHORT port)
{
	SUS_PRINTDL("Connecting to the server ...");
	SUS_ASSERT(addr && sct.sock);
	SOCKADDR_IN sockAddr = {
		.sin_addr.s_addr = inet_addr(addr),
		.sin_family = AF_INET,
		.sin_port = htons(port)
	};
	if (connect(sct.sock, (SOCKADDR*)&sockAddr, (int)sizeof(sockAddr)) == SOCKET_ERROR) {
		sct.handler(sct.sock, SUS_SOCK_ERROR, WSAGetLastError(), SUS_SOCK_CREATE);
		SUS_PRINTDE("Couldn't connect to the server");
		SUS_PRINTDC(WSAGetLastError());
		return (SOCKADDR_IN) { 0 };
	}
	SUS_PRINTDL("Successful connection to the server");
	return sockAddr;
}

// Create a server
SUS_SOCKET_CONTEXT SUSAPI susCreateServer(SUSNET_HANDLER handler)
{
	SUS_PRINTDL("Creating a server");
	SUS_ASSERT(handler);
	SUS_SOCKET_CONTEXT sct = susCreateSocket(handler);
	if (!susBuildSocket(&sct)) {
		SUS_PRINTDE("Failed to create a server");
		return (SUS_SOCKET_CONTEXT) { .sock = INVALID_SOCKET, .handler = NULL };
	}
	susSetSocketReuseAddr(sct.sock, TRUE);
	susSetSocketTimeout(sct.sock, 5);
	susSetSocketNoDelay(sct.sock, TRUE);
	SUS_PRINTDL("The server has been successfully created!");
	return sct;
}
// Start the server
BOOL SUSAPI susServerListen(_Inout_ SUS_SOCKET_CONTEXT sct, _In_ USHORT port)
{
	SUS_PRINTDL("Starting the server");
	if (sct.sock == INVALID_SOCKET) return FALSE;
	if (!susSetSocketLinger(sct.sock, TRUE, 5)) {
		SUS_PRINTDE("Couldn't start the server");
		return FALSE;
	}
	if (!susBindSocket(sct, port)) {
		SUS_PRINTDE("Couldn't start the server");
		return FALSE;
	}
	if (!susListenSocket(sct)) {
		SUS_PRINTDE("Couldn't start the server");
		return FALSE;
	}
	SUS_PRINTDL("The server has been successfully launched on port %d!", port);
	return TRUE;
}