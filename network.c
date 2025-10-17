// network.c
//
#include "coreframe.h"
#include "include/susfwk/core.h"
#include "include/susfwk/memory.h"
#include "include/susfwk/vector.h"
#include "include/susfwk/network.h"

//////////////////////////////////////////////////////////////////////////////////////////////////////
//									Deinitializing the library										//
//////////////////////////////////////////////////////////////////////////////////////////////////////

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

//////////////////////////////////////////////////////////////////////////////////////////////////////
//									Basic socket operations											//
//////////////////////////////////////////////////////////////////////////////////////////////////////

// Configure the base socket
SUS_SOCKET_STRUCT SUSAPI susSocketSetup(_In_opt_ SUS_SOCKET_HANDLER handler, _In_opt_ SUS_OBJECT userData)
{
	SUS_PRINTDL("Configuring the basic socket");
	return (SUS_SOCKET_STRUCT) {
		.sock = INVALID_SOCKET,
		.readBuffer = susNewBuffer(SUS_SOCKET_INIT_READ_BUFFER_SIZE),
		.writeBuffer = susNewBuffer(SUS_SOCKET_INIT_WRITE_BUFFER_SIZE),
		.handler = handler,
		.userdata = userData
	};
}
// Build a socket
BOOL SUSAPI susBuildSocket(_In_ SUS_SOCKET sock)
{
	SUS_PRINTDL("Socket registration");
	SUS_ASSERT(sock);
	sock->sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (sock->sock == INVALID_SOCKET) {
		if (sock->handler) sock->handler(sock, SUS_SM_ERROR, (WPARAM)WSAGetLastError(), (LPARAM)SUS_SM_CREATE);
		SUS_PRINTDE("Failed to register a socket in the system");
		SUS_PRINTDC(WSAGetLastError());
		return FALSE;
	}
	susSocketTunePerformance(sock);
	if (sock->handler) sock->handler(sock, SUS_SM_CREATE, 0, (LPARAM)sock->userdata);
	SUS_PRINTDL("The socket has been successfully registered!");
	return TRUE;
}
// Clean the socket
VOID SUSAPI susSocketCleanup(_Inout_ SUS_SOCKET sock)
{
	SUS_PRINTDL("Clearing the socket");
	SUS_ASSERT(sock);
	if (sock->readBuffer) {
		susBufferDestroy(sock->readBuffer);
		sock->readBuffer = NULL;
	}
	if (sock->writeBuffer) {
		susBufferDestroy(sock->writeBuffer);
		sock->writeBuffer = NULL;
	}
	sock->sock = INVALID_SOCKET;
	if (sock->handler) sock->handler(sock, SUS_SM_END, 0, 0);
}
// Force connection closure
BOOL SUSAPI susSocketClose(_In_ SUS_SOCKET sock)
{
	SUS_PRINTDL("Closing a network socket");
	SUS_ASSERT(sock && sock->sock != INVALID_SOCKET);
	if (sock->handler) sock->handler(sock, SUS_SM_CLOSE, 0, 0);
	if (closesocket(sock->sock) == SOCKET_ERROR) {
		if (sock->handler) sock->handler(sock, SUS_SM_ERROR, WSAGetLastError(), SUS_SM_END);
		susSocketCleanup(sock);
		SUS_PRINTDE("Couldn't close network socket");
		SUS_PRINTDC(WSAGetLastError());
		return FALSE;
	}
	susSocketCleanup(sock);
	SUS_PRINTDL("The network socket has been successfully closed");
	return TRUE;
}
// Standard connection closure
BOOL SUSAPI susSocketShutdown(_In_ SUS_SOCKET sock)
{
	SUS_PRINTDL("Secure connection closure");
	SUS_ASSERT(sock && sock->sock != INVALID_SOCKET);
	if (sock->handler) sock->handler(sock, SUS_SM_CLOSE, 0, 0);
	for (sus_uint i = 0; i < 10 && susSocketFlush(sock) == WSAEWOULDBLOCK; i++) Sleep(10);
	if (shutdown(sock->sock, SD_SEND) == SOCKET_ERROR) {
		SUS_PRINTDE("Couldn't close network socket");
		SUS_PRINTDC(WSAGetLastError());
		return FALSE;
	}
	Sleep(50);
	if (closesocket(sock->sock) == SOCKET_ERROR) {
		SUS_PRINTDE("Couldn't close network socket");
		SUS_PRINTDC(WSAGetLastError());
		return FALSE;
	}
	susSocketCleanup(sock);
	SUS_PRINTDL("The connection is safely closed");
	return TRUE;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
//									Low-level operations											//
//////////////////////////////////////////////////////////////////////////////////////////////////////

// Binding an address to a socket
BOOL SUSAPI susSocketBind(_In_ SUS_SOCKET sock, _In_ USHORT port)
{
	SUS_PRINTDL("Binding an address to a socket");
	SUS_ASSERT(sock->sock != INVALID_SOCKET);
	SOCKADDR_IN sockAddr = susSocketAddress("0.0.0.0", port);
	if (bind(sock->sock, (SOCKADDR*)&sockAddr, (int)sizeof(sockAddr)) == SOCKET_ERROR) {
		if (sock->handler) sock->handler(sock, SUS_SM_ERROR, WSAGetLastError(), 0);
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
	SUS_ASSERT(sock->sock != INVALID_SOCKET);
	if (listen(sock->sock, SOMAXCONN) == SOCKET_ERROR) {
		if (sock->handler) sock->handler(sock, SUS_SM_ERROR, WSAGetLastError(), 0);
		SUS_PRINTDE("Socket listening could not be set");
		SUS_PRINTDC(WSAGetLastError());
		return FALSE;
	}
	SUS_PRINTDL("The server listens for incoming connections");
	return TRUE;
}
// Accept connection
BOOL SUSAPI susSocketAccept(_In_ SUS_SERVER_SOCKET server, _Out_ SUS_SOCKET client, _Out_opt_ LPSOCKADDR_IN pAddr)
{
	SUS_PRINTDL("Accepting the connection");
	SUS_ASSERT(server && server->super.sock != INVALID_SOCKET && client);
	*client = susSocketSetup(server->heirHandler, server->super.userdata);
	SOCKADDR_IN addr = { 0 };
	INT size = sizeof(addr);
	client->sock = accept(server->super.sock, (SOCKADDR*)&addr, &size);
	if (client->sock == INVALID_SOCKET) {
		if (client->handler) client->handler(client, SUS_SM_ERROR, WSAGetLastError(), SUS_SM_CREATE);
		susSocketCleanup(client);
		if (pAddr) *pAddr = (SOCKADDR_IN){ 0 };
		SUS_PRINTDE("Couldn't accept connection");
		SUS_PRINTDC(WSAGetLastError());
		return FALSE;
	}
	susSocketTunePerformance(client);
	susSocketSetNonBlocking(client, TRUE);
	if (pAddr) *pAddr = addr;
	if (server->super.handler) server->super.handler(server, SUS_SM_START, (WPARAM)&addr, (LPARAM)client);
	if (client->handler) client->handler(client, SUS_SM_CREATE, (WPARAM)server, (LPARAM)server->super.userdata);
	if (client->handler) client->handler(client, SUS_SM_START, (WPARAM)server, (LPARAM)&addr);
	return TRUE;
}
// Connects to the server
BOOL SUSAPI susSocketConnect(_In_ SUS_SOCKET sock, _In_ LPCSTR addr, _In_ USHORT port)
{
	SUS_PRINTDL("Connecting to the server ...");
	SUS_ASSERT(sock && addr && sock->sock != INVALID_SOCKET);
	SOCKADDR_IN sockAddr = susSocketAddress(addr, port);
	if (connect(sock->sock, (SOCKADDR*)&sockAddr, (int)sizeof(sockAddr)) == SOCKET_ERROR) {
		if (sock->handler) sock->handler(sock, SUS_SM_ERROR, WSAGetLastError(), SUS_SM_START);
		SUS_PRINTDE("Couldn't connect to the server");
		SUS_PRINTDC(WSAGetLastError());
		return FALSE;
	}
	susSocketTunePerformance(sock);
	susSocketSetNonBlocking(sock, TRUE);
	if (sock->handler) sock->handler(sock, SUS_SM_START, (WPARAM)addr, port);
	SUS_PRINTDL("Successful connection to the server");
	return TRUE;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
//											I/O operations											//
//////////////////////////////////////////////////////////////////////////////////////////////////////

// Get data from a socket
BOOL SUSAPI susSocketRead(_Inout_ SUS_SOCKET sock)
{
	SUS_ASSERT(sock && sock->sock != INVALID_SOCKET);
	BYTE chunkBuffer[SUS_SOCKET_CHUNK_BUFFER_SIZE] = { 0 };
	INT bytesRead;
	do {
		bytesRead = recv(sock->sock, (PCHAR)chunkBuffer, (INT)sizeof(chunkBuffer), 0);
		if (!bytesRead) {
			susSocketShutdown(sock);
			return FALSE;
		}
		if (bytesRead == SOCKET_ERROR) {
			INT err = WSAGetLastError();
			if (err == WSAEWOULDBLOCK) return FALSE;
			if (sock->handler) sock->handler(sock, SUS_SM_ERROR, (WPARAM)err, 0);
			return FALSE;
		}
		susBufferAppend(&sock->readBuffer, chunkBuffer, bytesRead);
	} while (bytesRead == sizeof(chunkBuffer));
	if (sock->handler) sock->handler(sock, SUS_SM_DATA, (WPARAM)sock->readBuffer->size, (LPARAM)sock->readBuffer->data);
	susBufferClear(sock->readBuffer);
	return TRUE;
}
// Flushing the send buffer
SIZE_T SUSAPI susSocketFlush(_Inout_ SUS_SOCKET sock)
{
	SUS_ASSERT(sock && sock->sock != INVALID_SOCKET);
	while (sock->writeBuffer->size) {
		INT bytesWrite = send(sock->sock, (PCHAR)sock->writeBuffer->data, (INT)sock->writeBuffer->size, 0);
		if (bytesWrite == SOCKET_ERROR) {
			INT err = WSAGetLastError();
			if (err == WSAEWOULDBLOCK) return sock->writeBuffer->size;
			if (sock->handler) sock->handler(sock, SUS_SM_ERROR, (WPARAM)err, (LPARAM)sock->writeBuffer->size);
			return sock->writeBuffer->size;
		}
		susBufferErase(&sock->writeBuffer, 0, bytesWrite);
	};
	return 0;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
//										Working with the server										//
//////////////////////////////////////////////////////////////////////////////////////////////////////

// Create a server
SUS_SERVER_SOCKET_STRUCT SUSAPI susServerSetup(_In_opt_ SUS_SOCKET_HANDLER handler, _In_opt_ SUS_SOCKET_HANDLER heirHandler, _In_opt_ SUS_OBJECT userData)
{
	SUS_PRINTDL("Creating a server");
	return (SUS_SERVER_SOCKET_STRUCT) {
		.super = susSocketSetup(handler, userData),
		.clients = susNewVector(SUS_SOCKET_STRUCT),
		.clientfds = susNewVector(WSAPOLLFD),
		.heirHandler = heirHandler
	};
}
// Set the socket as a listening server
BOOL SUSAPI susServerListen(_Inout_ SUS_SERVER_SOCKET server, _In_ USHORT port)
{
	SUS_PRINTDL("Starting the server");
	if (!susBuildSocket(&server->super)) {
		SUS_PRINTDE("Couldn't start the server");
		return FALSE;
	}
	if (!susSocketBind(&server->super, port)) {
		SUS_PRINTDE("Couldn't start the server");
		return FALSE;
	}
	if (!susSocketListen(&server->super)) {
		SUS_PRINTDE("Couldn't start the server");
		return FALSE;
	}
	susSocketSetNonBlocking(&server->super, TRUE);
	SUS_PRINTDL("The server is running successfully! Server is listening for incoming connections on port: %d", port);
	return TRUE;
}
// Write the client to the server
static SUS_SOCKET SUSAPI susServerAddClient(_Inout_ SUS_SERVER_SOCKET server, _In_ SUS_SOCKET client) {
	WSAPOLLFD fd = { .fd = client->sock, .events = POLLIN | POLLOUT, .revents = 0 };
	susVectorPushBack(&server->clientfds, &fd);
	return susVectorPushBack(&server->clients, client);
}
// Delete the client from the server
static VOID SUSAPI susServerRemoveClient(_Inout_ SUS_SERVER_SOCKET server, _In_ UINT clientId) {
	susVectorErase(&server->clientfds, clientId);
	susVectorErase(&server->clients, clientId);
}
// Accepting the client to the server
SUS_SOCKET SUSAPI susServerAccept(_In_ SUS_SERVER_SOCKET server)
{
	SUS_PRINTDL("Accepting the connection");
	SUS_ASSERT(server && server->super.sock != INVALID_SOCKET);
	SUS_SOCKET_STRUCT client;
	if (!susSocketAccept(server, &client, NULL)) {
		SUS_PRINTDE("Failed to accept the client to the server");
		return NULL;
	}
	return susServerAddClient(server, &client);
}
// Cleaning up server resources
VOID SUSAPI susServerCleanup(_Inout_ SUS_SERVER_SOCKET server)
{
	susVecForeach(i, server->clients) { susSocketClose(susVectorGet(server->clients, i));  }
	susVectorDestroy(server->clientfds);
	susVectorDestroy(server->clients);
	susSocketCleanup(&server->super);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
//										Socket Managers												//
//////////////////////////////////////////////////////////////////////////////////////////////////////

// Process the socket
BOOL SUSAPI susSocketUpdate(_In_ SUS_SOCKET sock)
{
	SUS_ASSERT(sock && sock->sock != INVALID_SOCKET);
	WSAPOLLFD fd = { 0 };
	fd.fd = sock->sock;
	fd.events = POLLIN | POLLOUT;
	switch (WSAPoll(&fd, 1, SUS_SOCKET_POLL_TIMEOUT))
	{
	case SOCKET_ERROR: {
		if (sock->handler) sock->handler(sock, SUS_SM_ERROR, (WPARAM)WSAGetLastError(), SOCKET_ERROR);
		SUS_PRINTDE("Failed to poll sockets");
		SUS_PRINTDC(WSAGetLastError());
	} return FALSE;
	case 0: return TRUE;
	}
	if (fd.revents & POLLIN) {
		susSocketRead(sock);
	}
	if (fd.revents & POLLOUT) {
		susSocketFlush(sock);
	}
	if (fd.revents & POLLERR) {
		if (sock->handler) sock->handler(sock, SUS_SM_ERROR, (WPARAM)WSAGetLastError(), 0);
		if (sock->sock != INVALID_SOCKET) susSocketShutdown(sock);
		else susSocketCleanup(sock);
		return FALSE;
	}
	if (fd.revents & POLLHUP) {
		susSocketClose(sock);
		return FALSE;
	}
	return TRUE;
}
// Server Client Processing
static BOOL SUSAPI susClientsPoll(_Inout_ SUS_SERVER_SOCKET server)
{
	SUS_ASSERT(server && server->clients && server->clientfds && server->super.sock != INVALID_SOCKET);
	if (!server->clientfds->size) return TRUE;
	switch (WSAPoll((LPWSAPOLLFD)server->clientfds->data, server->clientfds->length, SUS_SOCKET_POLL_TIMEOUT))
	{
	case SOCKET_ERROR: {
		SUS_PRINTDE("Failed to poll sockets");
		SUS_PRINTDC(WSAGetLastError());
	} return FALSE;
	case 0: return TRUE;
	}
	susVecForeach(i, server->clientfds) {
		LPWSAPOLLFD fds = susVectorGet(server->clientfds, i);
		SUS_SOCKET client = susVectorGet(server->clients, i);
		if (fds->revents & POLLIN) {
			susSocketRead(client);
		}
		if (fds->revents & POLLOUT) {
			susSocketFlush(client);
		}
		if (fds->revents & POLLERR) {
			if (client->handler) client->handler(client, SUS_SM_ERROR, (WPARAM)WSAGetLastError(), POLLERR);
			if (client->sock != INVALID_SOCKET) susSocketShutdown(client);
			susServerRemoveClient(server, i);
			continue;
		}
		if (fds->revents & POLLHUP) {
			susSocketClose(client);
			susServerRemoveClient(server, i);
		}
	}
	return TRUE;
}
// Update the server status
BOOL SUSAPI susServerUpdate(_In_ SUS_SERVER_SOCKET server)
{
	SUS_ASSERT(server);
	WSAPOLLFD fd = {
		.fd = server->super.sock,
		.events = POLLRDNORM,
		.revents = 0
	};
	switch (WSAPoll(&fd, 1, SUS_SOCKET_POLL_TIMEOUT))
	{
	case SOCKET_ERROR: {
		if (server->super.handler) server->super.handler(&server->super, SUS_SM_ERROR, (WPARAM)WSAGetLastError(), SOCKET_ERROR);
		SUS_PRINTDE("Failed to poll sockets");
		SUS_PRINTDC(WSAGetLastError());
	} return FALSE;
	case 0: return susClientsPoll(server);
	}
	if (fd.revents & POLLRDNORM) {
		susServerAccept(server);
	}
	if (fd.revents & POLLERR) {
		if (server->super.handler) server->super.handler(&server->super, SUS_SM_ERROR, (WPARAM)WSAGetLastError(), 0);
		if (server->super.sock != INVALID_SOCKET) susSocketShutdown(&server->super);
	}
	if (fd.revents & POLLHUP) {
		susSocketClose(&server->super);
	}
	if (server->super.sock == INVALID_SOCKET) {
		susServerCleanup(server);
		return FALSE;
	}
	return susClientsPoll(server);
}
