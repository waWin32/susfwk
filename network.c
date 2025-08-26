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
SUS_SOCKET SUSAPI susBuildSocket(_In_ SUS_SOCKET_HANDLER handler, _In_opt_ LPVOID userData)
{
	SUS_PRINTDL("Building a socket");
	SUS_ASSERT(handler);
	SUS_SOCKET sock = susInitSocket(handler);
	sock.handler(&sock, SUS_SOCK_NCCREATE, (LPARAM)userData, IPPROTO_TCP);
	sock.sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (sock.sock == INVALID_SOCKET) {
		sock.handler(&sock, SUS_SOCK_ERROR, WSAGetLastError(), 0);
		SUS_PRINTDE("Couldn't build socket");
		SUS_PRINTDC(WSAGetLastError());
		return sock;
	}
	susSetSocketNonBlocking(sock.sock, TRUE);
	sock.handler(&sock, SUS_SOCK_CREATE, (LPARAM)userData, IPPROTO_TCP);
	SUS_PRINTDL("The socket has been successfully installed!");
	return sock;
}
// Closing a network socket
BOOL SUSAPI susSocketClose(_In_ SUS_LPSOCKET sock)
{
	SUS_PRINTDL("Closing a network socket");
	SUS_ASSERT(sock->sock);
	sock->handler(&sock, SUS_SOCK_CLOSE, 0, 0);
	if (closesocket(sock->sock) == SOCKET_ERROR) {
		sock->handler(&sock, SUS_SOCK_ERROR, WSAGetLastError(), 0);
		SUS_PRINTDE("Couldn't close network socket");
		SUS_PRINTDC(WSAGetLastError());
		return FALSE;
	}
	sock->sock = INVALID_SOCKET;
	sock->handler(&sock, SUS_SOCK_END, (LPARAM)sock, 0);
	SUS_PRINTDL("The network socket has been successfully closed");
	return TRUE;
}
// Binding an address to a socket
BOOL SUSAPI susSocketBind(_In_ SUS_SOCKET sock, _In_ USHORT port)
{
	SUS_PRINTDL("Binding an address to a socket");
	SUS_ASSERT(sock.sock);
	susSetSocketLinger(sock.sock, TRUE, 10);
	SOCKADDR_IN sockAddr = {
		.sin_addr.s_addr = INADDR_ANY,
		.sin_family = AF_INET,
		.sin_port = htons(port)
	};
	if (bind(sock.sock, (SOCKADDR*)&sockAddr, (int)sizeof(sockAddr)) == SOCKET_ERROR) {
		sock.handler(&sock, SUS_SOCK_ERROR, WSAGetLastError(), 0);
		SUS_PRINTDE("Failed to bind the socket to an address");
		SUS_PRINTDC(WSAGetLastError());
		return FALSE;
	}
	SUS_PRINTDL("The socket has been successfully bound to the address");
	return TRUE;
}
// Install Socket listening
BOOL SUSAPI susSocketListen(_In_ SUS_SOCKET sock)
{
	SUS_PRINTDL("Installing Socket listening");
	SUS_ASSERT(sock.sock);
	if (listen(sock.sock, SOMAXCONN) == SOCKET_ERROR) {
		sock.handler(&sock, SUS_SOCK_ERROR, WSAGetLastError(), 0);
		SUS_PRINTDE("Socket listening could not be set");
		SUS_PRINTDC(WSAGetLastError());
		return FALSE;
	}
	SUS_PRINTDL("The server listens for incoming connections");
	return TRUE;
}
// Accept connection
SUS_SOCKET SUSAPI susSocketAccept(_In_ SUS_SOCKET server, _In_ SUS_SOCKET_HANDLER handler, _Out_opt_ PSOCKADDR_IN addr, _In_opt_ LPVOID userData)
{
	SUS_PRINTDL("Accepting the connection");
	SUS_ASSERT(server.sock);
	SUS_SOCKET sock = susInitSocket(handler);
	INT size = sizeof(*addr);
	sock.handler(&sock, SUS_SOCK_NCCREATE, (LPARAM)userData, (WPARAM)server.sock);
	sock.sock = accept(server.sock, (SOCKADDR*)addr, &size);
	if (sock.sock == INVALID_SOCKET) {
		sock.handler(&sock, SUS_SOCK_ERROR, WSAGetLastError(), 0);
		SUS_PRINTDE("Couldn't accept connection");
		SUS_PRINTDC(WSAGetLastError());
		return sock;
	}
	susSetSocketNonBlocking(sock.sock, TRUE);
	susSetSocketTimeout(sock.sock, 5);
	sock.handler(&sock, SUS_SOCK_CREATE, (LPARAM)userData, (WPARAM)server.sock);
	sock.handler(&server, SUS_SOCK_ACCEPT, (LPARAM)addr, (WPARAM)sock.sock);
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
		sock.handler(&sock, SUS_SOCK_ERROR, WSAGetLastError(), 0);
		SUS_PRINTDE("Couldn't connect to the server");
		SUS_PRINTDC(WSAGetLastError());
		return (SOCKADDR_IN) { 0 };
	}
	SUS_PRINTDL("Successful connection to the server");
	sock.handler(&sock, SUS_SOCK_START, (LPARAM)addr, port);
	return sockAddr;
}

// Create a server
SUS_SOCKET SUSAPI susCreateServer(SUS_SOCKET_HANDLER handler, _In_opt_ LPVOID userData)
{
	SUS_PRINTDL("Creating a server");
	SUS_ASSERT(handler);
	SUS_SOCKET sock = susBuildSocket(handler, userData);
	if (!sock.sock) {
		SUS_PRINTDE("Failed to create a server");
		return (SUS_SOCKET) { .sock = INVALID_SOCKET, .handler = NULL };
	}
	susSetSocketReuseAddr(sock.sock, TRUE);
	susSetSocketTimeout(sock.sock, 5);
	susSetSocketNoDelay(sock.sock, TRUE);
	SUS_PRINTDL("The server has been successfully created!");
	return sock;
}
// Start the server
BOOL SUSAPI susServerListen(_Inout_ SUS_SOCKET sock, _In_ USHORT port)
{
	SUS_PRINTDL("Starting the server");
	if (sock.sock == INVALID_SOCKET) return FALSE;
	if (!susSetSocketLinger(sock.sock, TRUE, 5)) {
		SUS_PRINTDE("Couldn't start the server");
		return FALSE;
	}
	if (!susSocketBind(sock, port)) {
		SUS_PRINTDE("Couldn't start the server");
		return FALSE;
	}
	if (!susSocketListen(sock)) {
		SUS_PRINTDE("Couldn't start the server");
		return FALSE;
	}
	SUS_PRINTDL("The server has been successfully launched on port %d!", port);
	sock.handler(&sock, SUS_SOCK_START, 0, port);
	return TRUE;
}

// Polling socket events
BOOL SUSAPI susSocketPollEvents(_Inout_ SUS_LPSOCKET sock)
{
	SUS_ASSERT(sock);
	if (sock->sock == INVALID_SOCKET) return FALSE;
	WSAPOLLFD pollFd = {
		.fd = sock->sock,
		.events = POLLRDNORM | POLLERR | POLLHUP,
		.revents = 0
	};
	if (sock->writeBuffer->size) pollFd.events |= POLLWRNORM;
	switch (WSAPoll(&pollFd, 1, SUS_SOCKET_POLL_TIMEOUT)) {
	case SOCKET_ERROR:
		sock->handler(&sock, SUS_SOCK_ERROR, (LPARAM)WSAGetLastError(), 0);
		return FALSE;
	case 0:
		sock->handler(&sock, SUS_SOCK_TIMEOUT, 0, 0);
		return FALSE;
	}
	if (pollFd.revents & (POLLERR | POLLHUP)) {
		sock->handler(&sock, SUS_SOCK_ERROR, (LPARAM)susGetSocketError(sock->sock), 0);
		susSocketClose(sock);
	}
	if (pollFd.revents & POLLRDNORM) {
		if (!susSocketRead(sock)) return FALSE;
	}
	if ((pollFd.revents & POLLWRBAND) && sock->writeBuffer->size) {
		susSocketFlush(sock);
	}
	return TRUE;
}