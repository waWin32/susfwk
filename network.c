// network.c
//
#include "coreframe.h"
#include "include/susfwk/core.h"
#include "include/susfwk/memory.h"
#include "include/susfwk/vector.h"
#include "include/susfwk/hashtable.h"
#include "include/susfwk/network.h"

#pragma warning(disable: 28159)

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
SUS_SOCKET SUSAPI susSocketSetup(_In_opt_ SUS_SOCKET_HANDLER handler, _In_opt_ SUS_OBJECT userData)
{
	SUS_PRINTDL("Configuring the basic socket");
	return (SUS_SOCKET) {
		.sock = INVALID_SOCKET,
		.readBuffer = susNewBuffer(SUS_SOCKET_INIT_READ_BUFFER_SIZE),
		.writeBuffer = susNewBuffer(SUS_SOCKET_INIT_WRITE_BUFFER_SIZE),
		.handler = handler,
		.timers = susNewVector(SUS_SOCKET_TIMER),
		.userData = userData
	};
}
// Build a socket
BOOL SUSAPI susBuildSocket(_In_ SUS_LPSOCKET sock)
{
	SUS_PRINTDL("Socket registration");
	SUS_ASSERT(sock);
	sock->sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (sock->sock == INVALID_SOCKET) {
		SUS_PRINTDE("Failed to register a socket in the system");
		SUS_PRINTDC(WSAGetLastError());
		SUS_ASSERT(WSAGetLastError() != 10093); // You need to initialize the WinSocket library
		if (sock->handler) sock->handler(sock, SUS_SM_ERROR, (WPARAM)WSAGetLastError(), (LPARAM)SUS_SOCKET_ERROR_FAILED_CREATE);
		return FALSE;
	}
	if (sock->handler) if (!sock->handler(sock, SUS_SM_CREATE, 0, (LPARAM)sock->userData)) {
		SUS_PRINTDL("The user's CALLBACK decided to forcefully terminate the socket creation");
		closesocket(sock->sock);
		return FALSE;
	}
	SUS_PRINTDL("The socket has been successfully registered!");
	return TRUE;
}
// Clean the socket
VOID SUSAPI susSocketCleanup(_Inout_ SUS_LPSOCKET sock)
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
	if (sock->timers) {
		susVectorDestroy(sock->timers);
		sock->timers = NULL;
	}
	sock->sock = INVALID_SOCKET;
	if (sock->handler) sock->handler(sock, SUS_SM_END, 0, 0);
}
// Force connection closure
BOOL SUSAPI susSocketClose(_Inout_ SUS_LPSOCKET sock)
{
	SUS_PRINTDL("Closing a network socket");
	SUS_ASSERT(sock && sock->sock != INVALID_SOCKET);
	if (closesocket(sock->sock) == SOCKET_ERROR) {
		if (sock->handler) sock->handler(sock, SUS_SM_ERROR, WSAGetLastError(), SUS_SOCKET_ERROR_FAILED_CLOSE);
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
BOOL SUSAPI susSocketShutdown(_Inout_ SUS_LPSOCKET sock)
{
	SUS_PRINTDL("Secure connection closure");
	SUS_ASSERT(sock && sock->sock != INVALID_SOCKET);
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
		susSocketCleanup(sock);
		return FALSE;
	}
	susSocketCleanup(sock);
	SUS_PRINTDL("The connection is safely closed");
	return TRUE;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
//									Working with a socket timer										//
//////////////////////////////////////////////////////////////////////////////////////////////////////

// Set a timer for the socket
BOOL SUSAPI susSocketSetTimer(_Inout_ SUS_LPSOCKET sock, _In_ DWORD interval, _In_ DWORD id)
{
	SUS_PRINTDL("Setting a timer on a socket");
	SUS_ASSERT(sock && sock->timers && interval);
	SUS_SOCKET_TIMER timer = { .id = id, .interval = interval, .nextFire = GetTickCount() + interval };
	return susVectorPushBack(&sock->timers, &timer) ? TRUE : FALSE;
}
// Delete the socket timer
VOID SUSAPI susSocketKillTimer(_Inout_ SUS_LPSOCKET sock, _In_ DWORD id)
{
	SUS_PRINTDL("Removing the timer from the socket");
	SUS_ASSERT(sock && sock->timers);
	susVectorErase(&sock->timers, id);
}
// Running all suitable socket timers
static VOID SUSAPI susSocketTimersStart(_In_ SUS_LPSOCKET sock) {
	if (!sock->handler) return;
	DWORD currTime = GetTickCount();
	susVecForeach(i, sock->timers) {
		SUS_LPSOCKET_TIMER timer = susVectorGet(sock->timers, i);
		if (currTime >= timer->nextFire) {
			sock->handler(sock, SUS_SM_TIMER, timer->id, timer->interval);
			timer->nextFire = currTime + timer->interval;
		}
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
//									Low-level operations											//
//////////////////////////////////////////////////////////////////////////////////////////////////////

// Binding an address to a socket
BOOL SUSAPI susSocketBind(_In_ SUS_LPSOCKET sock, _In_ USHORT port)
{
	SUS_PRINTDL("Binding an address to a socket");
	SUS_ASSERT(sock->sock != INVALID_SOCKET);
	sock->addr = susSocketAddress("0.0.0.0", port);
	if (bind(sock->sock, (SOCKADDR*)&sock->addr, (int)sizeof(sock->addr)) == SOCKET_ERROR) {
		if (sock->handler) sock->handler(sock, SUS_SM_ERROR, WSAGetLastError(), SUS_SOCKET_ERROR_FAILED_BIND);
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
	SUS_ASSERT(sock->sock != INVALID_SOCKET);
	if (listen(sock->sock, SOMAXCONN) == SOCKET_ERROR) {
		if (sock->handler) sock->handler(sock, SUS_SM_ERROR, WSAGetLastError(), SUS_SOCKET_ERROR_FAILED_LISTEN);
		SUS_PRINTDE("Socket listening could not be set");
		SUS_PRINTDC(WSAGetLastError());
		return FALSE;
	}
	SUS_PRINTDL("The server listens for incoming connections");
	return TRUE;
}
// Accept connection
BOOL SUSAPI susSocketAccept(_In_ SUS_LPSERVER_SOCKET server, _Out_ SUS_LPSOCKET client)
{
	SUS_PRINTDL("Accepting the connection");
	SUS_ASSERT(server && server->super.sock != INVALID_SOCKET && client);
	*client = susSocketSetup(server->heirHandler, server->super.userData);
	INT size = sizeof(client->addr);
	client->sock = accept(server->super.sock, (SOCKADDR*)&client->addr, &size);
	if (client->sock == INVALID_SOCKET) {
		SUS_PRINTDE("Couldn't accept connection");
		SUS_PRINTDC(WSAGetLastError());
		if (client->handler) client->handler(client, SUS_SM_ERROR, WSAGetLastError(), SUS_SOCKET_ERROR_FAILED_START);
		susSocketCleanup(client);
		return FALSE;
	}
	susSocketTunePerformance(client);
	susSocketSetNonBlocking(client, TRUE);
	if (server->super.handler) if (!server->super.handler(server, SUS_SM_START, (WPARAM)&client->addr, (LPARAM)client)) goto user_socket_shutdown;
	if (client->handler) {
		if (!client->handler(client, SUS_SM_CREATE, (WPARAM)server, (LPARAM)&client->addr)) goto user_socket_shutdown;
		if (!client->handler(client, SUS_SM_START, (WPARAM)server, (LPARAM)&client->addr)) goto user_socket_shutdown;
	}
	return TRUE;
user_socket_shutdown:
	SUS_PRINTDL("The user's CALLBACK decided to forcefully terminate the socket creation");
	susSocketShutdown(client);
	return FALSE;
}
// Connects to the server
BOOL SUSAPI susSocketConnect(_In_ SUS_LPSOCKET sock, _In_ LPCSTR addr, _In_ USHORT port)
{
	SUS_PRINTDL("Connecting to the server ...");
	SUS_ASSERT(sock && addr && sock->sock != INVALID_SOCKET);
	sock->addr = susSocketAddress(addr, port);
	if (connect(sock->sock, (SOCKADDR*)&sock->addr, (int)sizeof(sock->addr)) == SOCKET_ERROR) {
		SUS_PRINTDE("Couldn't connect to the server");
		SUS_PRINTDC(WSAGetLastError());
		if (sock->handler) sock->handler(sock, SUS_SM_ERROR, WSAGetLastError(), SUS_SOCKET_ERROR_FAILED_START);
		return FALSE;
	}
	susSocketTunePerformance(sock);
	susSocketSetNonBlocking(sock, TRUE);
	if (sock->handler && !sock->handler(sock, SUS_SM_START, (WPARAM)addr, port)) {
		SUS_PRINTDL("The user's CALLBACK decided to forcefully terminate the socket creation");
		closesocket(sock->sock);
		return FALSE;
	}
	SUS_PRINTDL("Successful connection to the server");
	return TRUE;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
//											I/O operations											//
//////////////////////////////////////////////////////////////////////////////////////////////////////

// Get data from a socket
BOOL SUSAPI susSocketRead(_Inout_ SUS_LPSOCKET sock)
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
			if (err == WSAEWOULDBLOCK) return TRUE;
			if (sock->handler) sock->handler(sock, SUS_SM_ERROR, (WPARAM)err, SUS_SOCKET_ERROR_FAILED_READ);
			return FALSE;
		}
		susBufferAppend(&sock->readBuffer, chunkBuffer, bytesRead);
		do {
			LPSTR endMessage = sus_strchrA((LPSTR)sock->readBuffer->data, '\0');
			if (!endMessage || (SIZE_T)((LPBYTE)endMessage - sock->readBuffer->data) >= sock->readBuffer->size) {
				SUS_PRINTDL("The message you read is incomplete");
				break;
			}
			SIZE_T dataSize = (SIZE_T)((LPBYTE)endMessage - sock->readBuffer->data);
			if (sock->handler && dataSize) sock->handler(sock, SUS_SM_DATA, (WPARAM)dataSize, (LPARAM)sock->readBuffer->data);
			susBufferErase(&sock->readBuffer, 0, dataSize + 1);
		} while (sock->readBuffer->size);
		if (susBufferSize(sock->readBuffer) > SUS_SOCKET_MAX_MESSAGE_SIZE) if (sock->handler && !sock->handler(sock, SUS_SM_ERROR, (WPARAM)0, SUS_SOCKET_ERROR_BUFFER_OVERFLOW)) {
			susSocketShutdown(sock);
			break;
		}
	} while (bytesRead == sizeof(chunkBuffer));
	return TRUE;
}
// Flushing the send buffer
SIZE_T SUSAPI susSocketFlush(_Inout_ SUS_LPSOCKET sock)
{
	SUS_ASSERT(sock && sock->sock != INVALID_SOCKET);
	if (sock->handler) sock->handler(sock, SUS_SM_WRITE, 0, 0);
	while (sock->writeBuffer->size) {
		INT bytesWrite = send(sock->sock, (PCHAR)sock->writeBuffer->data, (INT)sock->writeBuffer->size, 0);
		if (bytesWrite == SOCKET_ERROR) {
			INT err = WSAGetLastError();
			if (err == WSAEWOULDBLOCK) return sock->writeBuffer->size;
			if (sock->handler) sock->handler(sock, SUS_SM_ERROR, (WPARAM)err, (LPARAM)SUS_SOCKET_ERROR_FAILED_WRITE);
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
SUS_SERVER_SOCKET SUSAPI susServerSetup(_In_opt_ SUS_SOCKET_HANDLER handler, _In_opt_ SUS_SOCKET_HANDLER heirHandler, _In_opt_ SUS_OBJECT userData)
{
	SUS_PRINTDL("Creating a server");
	return (SUS_SERVER_SOCKET) {
		.super = susSocketSetup(handler, userData),
		.clients = susNewVector(SUS_SOCKET),
		.clientfds = susNewVector(WSAPOLLFD),
		.heirHandler = heirHandler
	};
}
// Set the socket as a listening server
BOOL SUSAPI susServerListen(_Inout_ SUS_LPSERVER_SOCKET server, _In_ USHORT port)
{
	SUS_PRINTDL("Starting the server");
	if (!susBuildSocket(&server->super)) {
		SUS_PRINTDE("Couldn't start the server");
		return FALSE;
	}
	if (!susSocketBind(&server->super, port)) {
		SUS_PRINTDE("Couldn't start the server");
		closesocket(server->super.sock);
		return FALSE;
	}
	if (!susSocketListen(&server->super)) {
		SUS_PRINTDE("Couldn't start the server");
		closesocket(server->super.sock);
		return FALSE;
	}
	susSocketTunePerformance(&server->super);
	susSocketSetNonBlocking(&server->super, TRUE);
	SUS_PRINTDL("The server is running successfully! Server is listening for incoming connections on port: %d", port);
	return TRUE;
}
// Write the client to the server
static SUS_LPSOCKET SUSAPI susServerAddClient(_Inout_ SUS_LPSERVER_SOCKET server, _In_ SUS_LPSOCKET client) {
	WSAPOLLFD fd = { .fd = client->sock, .events = POLLIN | POLLOUT, .revents = 0 };
	susVectorPushBack(&server->clientfds, &fd);
	return susVectorPushBack(&server->clients, client);
}
// Delete the client from the server
static VOID SUSAPI susServerRemoveClient(_Inout_ SUS_LPSERVER_SOCKET server, _In_ UINT clientId) {
	SUS_ASSERT(server);
	susVectorErase(&server->clientfds, clientId);
	susVectorErase(&server->clients, clientId);
}
// Accepting the client to the server
SUS_LPSOCKET SUSAPI susServerAccept(_In_ SUS_LPSERVER_SOCKET server)
{
	SUS_PRINTDL("Accepting the connection");
	SUS_ASSERT(server && server->super.sock != INVALID_SOCKET);
	SUS_SOCKET client;
	if (!susSocketAccept(server, &client)) {
		SUS_PRINTDE("Failed to accept the client to the server");
		return NULL;
	}
	return susServerAddClient(server, &client);
}
// Cleaning up server resources
VOID SUSAPI susServerCleanup(_Inout_ SUS_LPSERVER_SOCKET server)
{
	SUS_ASSERT(server);
	susVecForeach(i, server->clients) { susSocketClose(susVectorGet(server->clients, i));  }
	susVectorDestroy(server->clientfds);
	susVectorDestroy(server->clients);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
//										Socket Managers												//
//////////////////////////////////////////////////////////////////////////////////////////////////////

// Process the socket
BOOL SUSAPI susSocketUpdate(_In_ SUS_LPSOCKET sock)
{
	SUS_ASSERT(sock && sock->sock != INVALID_SOCKET);
	WSAPOLLFD fd = { 0 };
	fd.fd = sock->sock;
	fd.events = POLLIN | POLLOUT;
	switch (WSAPoll(&fd, 1, SUS_SOCKET_POLL_TIMEOUT))
	{
	case SOCKET_ERROR: {
		SUS_PRINTDE("Failed to poll sockets");
		SUS_PRINTDC(WSAGetLastError());
		if (sock->handler) sock->handler(sock, SUS_SM_ERROR, (WPARAM)WSAGetLastError(), SUS_SOCKET_ERROR_FAILED_POLL);
		susSocketClose(sock);
	} return FALSE;
	case 0: return TRUE;
	}
	if (fd.revents & POLLIN) {
		susSocketRead(sock);
	}
	if (fd.revents & POLLOUT) {
		susSocketTimersStart(sock);
		susSocketFlush(sock);
	}
	if (fd.revents & POLLERR) {
		if (sock->handler) sock->handler(sock, SUS_SM_ERROR, (WPARAM)WSAGetLastError(), SUS_SOCKET_ERROR_POLL);
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
static BOOL SUSAPI susClientsPoll(_Inout_ SUS_LPSERVER_SOCKET server)
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
	susVecForeachReverse(i, server->clientfds) {
		LPWSAPOLLFD fds = susVectorGet(server->clientfds, i);
		SUS_LPSOCKET client = susVectorGet(server->clients, i);
		if (fds->revents & POLLIN) {
			susSocketRead(client);
		}
		if (fds->revents & POLLOUT) {
			susSocketTimersStart(client);
			susSocketFlush(client);
		}
		if (fds->revents & POLLERR) {
			if (client->handler) client->handler(client, SUS_SM_ERROR, (WPARAM)WSAGetLastError(), SUS_SOCKET_ERROR_POLL);
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
BOOL SUSAPI susServerUpdate(_In_ SUS_LPSERVER_SOCKET server)
{
	SUS_ASSERT(server);
	WSAPOLLFD fd = {
		.fd = server->super.sock,
		.events = POLLRDNORM,
		.revents = 0
	};
	if (server->super.sock == INVALID_SOCKET) goto afterEvents;
	switch (WSAPoll(&fd, 1, SUS_SOCKET_POLL_TIMEOUT))
	{
	case SOCKET_ERROR: {
		SUS_PRINTDE("Failed to poll sockets");
		SUS_PRINTDC(WSAGetLastError());
		if (server->super.handler) server->super.handler(&server->super, SUS_SM_ERROR, (WPARAM)WSAGetLastError(), SUS_SOCKET_ERROR_FAILED_POLL);
		susSocketClose(&server->super);
		susServerCleanup(server);
	} goto afterEvents;
	case 0: goto afterEvents;
	}
	if (fd.revents & POLLRDNORM) {
		susServerAccept(server);
	}
	if (fd.revents & POLLERR) {
		if (server->super.handler) server->super.handler(&server->super, SUS_SM_ERROR, (WPARAM)WSAGetLastError(), SUS_SOCKET_ERROR_POLL);
		if (server->super.sock != INVALID_SOCKET) susSocketClose(&server->super);
	}
	if (fd.revents & POLLHUP) {
		susSocketClose(&server->super);
	}
afterEvents:
	if (server->super.sock == INVALID_SOCKET) {
		susServerCleanup(server);
		return FALSE;
	}
	return susClientsPoll(server);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//								      Formatting binary data									  //
////////////////////////////////////////////////////////////////////////////////////////////////////

// Format binary data into a string
LPSTR SUSAPI susBinToStr(_In_ SUS_DATAVIEW bin)
{
	SUS_BUFFER buff = susNewBuffer(bin.size);
	while (bin.size) {
		LPBYTE curr = (LPBYTE)sus_strchrA((LPCSTR)bin.data, '\0');
		if (curr >= bin.data + bin.size) {
			susBufferAppend(&buff, bin.data, bin.size);
			break;
		}
		susBufferAppend(&buff, bin.data, curr - bin.data);
		susBufferAppend(&buff, (LPBYTE)"\\0", 2);
		bin.size -= curr - bin.data + 1;
		bin.data = curr + 1;
	}
	susBufferAppend(&buff, (LPBYTE)"", sizeof(CHAR));
	LPSTR str = sus_strdup((LPCSTR)buff->data);
	susBufferDestroy(buff);
	return str;
}
// Format a string into binary data
SUS_DATAVIEW SUSAPI susStrToBin(_In_ LPCSTR str)
{
	SUS_BUFFER buff = susNewBuffer(lstrlenA(str));
	DWORD len = lstrlenA(str);
	while (*str) {
		LPCSTR curr = sus_strstrA(str, "\\0");
		if (!curr) {
			susBufferAppend(&buff, (LPBYTE)str, len);
			break;
		}
		susBufferAppend(&buff, (LPBYTE)str, (LPBYTE)curr - (LPBYTE)str);
		susBufferAppend(&buff, (LPBYTE)"", sizeof(BYTE));
		len -= (DWORD)(curr - str + 2);
		str = curr + 2;
	}
	SUS_DATAVIEW bin = susNewData(buff->size);
	sus_memcpy(bin.data, buff->data, buff->size);
	return bin;
}
