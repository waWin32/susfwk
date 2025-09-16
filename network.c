// network.c
//
#include "coreframe.h"
#include "include/susfwk/core.h"
#include "include/susfwk/memory.h"
#include "include/susfwk/vector.h"
#include "include/susfwk/linkedlist.h"
#include "include/susfwk/network.h"

// -------------------------------------------------------------------------------------------------------------

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

// -------------------------------------------------------------------------------------------------------------

// Configure the base socket
SUS_SOCKET SUSAPI susSocketSetup(_In_ SUS_SOCKET_HANDLER handler)
{
	SUS_PRINTDL("Configuring the basic socket");
	return (SUS_SOCKET) {
		.sock = INVALID_SOCKET,
		.readBuffer = susNewBuffer(SUS_SOCKET_READ_BUFFER_SIZE),
		.writeBuffer = susNewBuffer(SUS_SOCKET_WRITE_BUFFER_SIZE),
		.handler = handler
	};
}
// Clean the socket
VOID SUSAPI susSocketCleanup(_Inout_ SUS_LPSOCKET sock)
{
	SUS_PRINTDL("Clearing the socket");
	susBufferDestroy(sock->readBuffer);
	susBufferDestroy(sock->writeBuffer);
	sock->sock = INVALID_SOCKET;
}

// -------------------------------------------------------------------------------------------------------------

// Closing a network socket
BOOL SUSAPI susSocketClose(_In_ SUS_LPSOCKET sock)
{
	SUS_PRINTDL("Closing a network socket");
	SUS_ASSERT(sock->sock);
	sock->handler(&sock, SUS_SOCK_END, 0, 0);
	if (closesocket(sock->sock) == SOCKET_ERROR) {
		sock->handler(&sock, SUS_SOCK_ERROR, WSAGetLastError(), SUS_SOCK_END);
		SUS_PRINTDE("Couldn't close network socket");
		SUS_PRINTDC(WSAGetLastError());
		return FALSE;
	}
	sock->sock = INVALID_SOCKET;
	sock->handler(&sock, SUS_SOCK_CLOSE, 0, 0);
	susSocketCleanup(sock);
	SUS_PRINTDL("The network socket has been successfully closed");
	return TRUE;
}
// Binding an address to a socket
BOOL SUSAPI susSocketBind(_In_ SUS_LPSOCKET sock, _In_ USHORT port)
{
	SUS_PRINTDL("Binding an address to a socket");
	SUS_ASSERT(sock->sock);
	SOCKADDR_IN sockAddr = {
		.sin_addr.s_addr = INADDR_ANY,
		.sin_family = AF_INET,
		.sin_port = htons(port)
	};
	if (bind(sock->sock, (SOCKADDR*)&sockAddr, (int)sizeof(sockAddr)) == SOCKET_ERROR) {
		sock->handler(sock, SUS_SOCK_ERROR, WSAGetLastError(), 0);
		SUS_PRINTDE("Failed to bind the socket to an address");
		SUS_PRINTDC(WSAGetLastError());
		return FALSE;
	}
	SUS_PRINTDL("The socket has been successfully bound to the address");
	return TRUE;
}
// Install Socket listening
BOOL SUSAPI susSocketListen(_In_ SUS_LPSOCKET sock)
{
	SUS_PRINTDL("Installing Socket listening");
	SUS_ASSERT(sock->sock);
	if (listen(sock->sock, SOMAXCONN) == SOCKET_ERROR) {
		sock->handler(sock, SUS_SOCK_ERROR, WSAGetLastError(), 0);
		SUS_PRINTDE("Socket listening could not be set");
		SUS_PRINTDC(WSAGetLastError());
		return FALSE;
	}
	SUS_PRINTDL("The server listens for incoming connections");
	return TRUE;
}
// Accept connection
SUS_SOCKET SUSAPI susSocketAccept(_In_ SUS_LPSOCKET server, _In_ SUS_SOCKET_HANDLER handler, _Out_opt_ PSOCKADDR_IN addr)
{
	SUS_PRINTDL("Accepting the connection");
	SUS_ASSERT(server->sock);
	SUS_SOCKET sock = susSocketSetup(handler);
	INT size = sizeof(*addr);
	sock.sock = accept(server->sock, (SOCKADDR*)addr, &size);
	if (sock.sock == INVALID_SOCKET) {
		sock.handler(&sock, SUS_SOCK_ERROR, WSAGetLastError(), SUS_SOCK_CREATE);
		SUS_PRINTDE("Couldn't accept connection");
		SUS_PRINTDC(WSAGetLastError());
		return sock;
	}
	sock.handler(&sock, SUS_SOCK_CREATE, 0, (WPARAM)server->sock);
	sock.handler(&server, SUS_SOCK_START, (LPARAM)&sock, (WPARAM)addr);
	SUS_PRINTDL("Connection accepted");
	return sock;
}
// Connects to the server
SOCKADDR_IN SUSAPI susConnectToServer(_In_ SUS_SOCKET sock, _In_ LPCSTR addr, _In_ USHORT port)
{
	SUS_PRINTDL("Connecting to the server ...");
	SUS_ASSERT(addr && sock.sock);
	SOCKADDR_IN sockAddr = {
		.sin_addr.s_addr = inet_addr(addr),
		.sin_family = AF_INET,
		.sin_port = htons(port)
	};
	if (connect(sock.sock, (SOCKADDR*)&sockAddr, (int)sizeof(sockAddr)) == SOCKET_ERROR) {
		sock.handler(&sock, SUS_SOCK_ERROR, WSAGetLastError(), SUS_SOCK_START);
		SUS_PRINTDE("Couldn't connect to the server");
		SUS_PRINTDC(WSAGetLastError());
		return (SOCKADDR_IN) { 0 };
	}
	sock.handler(&sock, SUS_SOCK_START, (LPARAM)addr, port);
	SUS_PRINTDL("Successful connection to the server");
	return sockAddr;
}

// Get data from a socket
BOOL SUSAPI susSocketRead(_Inout_ SUS_LPSOCKET sock)
{
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
		susBufferAppend(&sock->readBuffer, chunkBuffer, bytesRead);
	} while (bytesRead == sizeof(chunkBuffer));
	sock->handler(&sock, SUS_SOCK_DATA, (LPARAM)sock->readBuffer->data, sock->readBuffer->size);
	susBufferClear(sock->readBuffer);
	return TRUE;
}
// Flushing the send buffer
BOOL SUSAPI susSocketFlush(_Inout_ SUS_LPSOCKET sock)
{
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
