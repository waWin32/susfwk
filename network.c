// network.c
//
#include "coreframe.h"
#include "include/susfwk/core.h"
#include "include/susfwk/thrprocessapi.h"
#include "include/susfwk/buffer.h"
#include "include/susfwk/vector.h"
#include "include/susfwk/hashtable.h"
#include "include/susfwk/network.h"

//////////////////////////////////////////////////////////////////////////////////////////////////////
//									Basic socket operations											//
//////////////////////////////////////////////////////////////////////////////////////////////////////

// -----------------------------------------------

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

// -----------------------------------------------

// Configure the base socket
SUS_SOCKET SUSAPI susSocketSetup(_In_opt_ SUS_SOCKET_HANDLER handler, _In_opt_ SUS_USERDATA userData)
{
	SUS_PRINTDL("Configuring the basic socket");
	return (SUS_SOCKET) {
		.super = INVALID_SOCKET,
			.buffers = {
				.readBuffer = susNewBuffer(SUS_SOCKET_INIT_BUFFER_SIZE),
				.writeBuffer = susNewBuffer(SUS_SOCKET_INIT_BUFFER_SIZE)
		},
		.active = FALSE,
		.handler = handler,
		.userData = userData
	};
}
// Build a socket
BOOL SUSAPI susBuildSocket(_In_ SUS_LPSOCKET sock, _In_ ADDRESS_FAMILY type)
{
	SUS_PRINTDL("Socket registration");
	SUS_ASSERT(sock && type);
	sock->super = socket(type, SOCK_STREAM, IPPROTO_TCP);
	if (sock->super == INVALID_SOCKET) {
		SUS_PRINTDE("Failed to register a socket in the system");
		SUS_PRINTDC(WSAGetLastError());
		SUS_ASSERT(WSAGetLastError() != 10093); // You need to initialize the WinSocket library
		susSocketCallMessage(sock, SUS_SM_ERROR, (WPARAM)WSAGetLastError(), (LPARAM)SUS_SOCKET_ERROR_FAILED_CREATE);
		return FALSE;
	}
	sock->active = TRUE;
	if (susSocketCallMessage(sock, SUS_SM_CREATE, 0, 0)) {
		SUS_PRINTDL("The user's CALLBACK decided to forcefully terminate the socket creation");
		closesocket(sock->super);
		return FALSE;
	}
	SUS_PRINTDL("The socket has been successfully registered!");
	return TRUE;
}

// -----------------------------------------------

// Clean the socket
static VOID SUSAPI susSocketCleanup(_Inout_ SUS_LPSOCKET sock)
{
	SUS_PRINTDL("Clearing the socket");
	SUS_ASSERT(sock);
	susSocketCallMessage(sock, SUS_SM_END, 0, 0);
	if (sock->buffers.readBuffer) {
		susBufferDestroy(sock->buffers.readBuffer);
		sock->buffers.readBuffer = NULL;
	}
	if (sock->buffers.writeBuffer) {
		susBufferDestroy(sock->buffers.writeBuffer);
		sock->buffers.writeBuffer = NULL;
	}
	if (sock->timers) {
		susMapDestroy(sock->timers);
		sock->timers = NULL;
	}
	if (sock->properties) {
		susMapForeach(sock->properties, i) sus_free(*(LPSTR*)susMapIterKey(i));
		susMapDestroy(sock->properties);
		sock->properties = NULL;
	}
}
// Force connection closure
static BOOL SUSAPI susSocketClose(_Inout_ SUS_LPSOCKET sock)
{
	SUS_PRINTDL("Closing a network socket");
	SUS_ASSERT(sock && sock->super != INVALID_SOCKET);
	susSocketCallMessage(sock, SUS_SM_CLOSE, 0, 0);
	if (closesocket(sock->super) == SOCKET_ERROR) {
		SUS_PRINTDE("Couldn't close network socket");
		SUS_PRINTDC(WSAGetLastError());
		susSocketCallMessage(sock, SUS_SM_ERROR, WSAGetLastError(), SUS_SOCKET_ERROR_FAILED_CLOSE);
		return FALSE;
	}
	sock->super = INVALID_SOCKET;
	susSocketCleanup(sock);
	SUS_PRINTDL("The network socket has been successfully closed");
	return TRUE;
}
// Standard connection closure
static BOOL SUSAPI susSocketShutdown(_Inout_ SUS_LPSOCKET sock)
{
	SUS_PRINTDL("Secure connection closure");
	SUS_ASSERT(sock && sock->super != INVALID_SOCKET);
	susSocketCallMessage(sock, SUS_SM_CLOSE, 0, 0);
	for (sus_uint_t i = 0; i < 10 && susSocketFlush(sock) == WSAEWOULDBLOCK; i++) Sleep(SUS_SOCKET_POLL_TIMEOUT);
	if (shutdown(sock->super, SD_SEND) == SOCKET_ERROR) {
		SUS_PRINTDE("Couldn't close network socket");
		SUS_PRINTDC(WSAGetLastError());
		susSocketCallMessage(sock, SUS_SM_ERROR, WSAGetLastError(), SUS_SOCKET_ERROR_FAILED_CLOSE);
		return FALSE;
	}
	if (closesocket(sock->super) == SOCKET_ERROR) {
		SUS_PRINTDE("Couldn't close network socket");
		SUS_PRINTDC(WSAGetLastError());
		susSocketCallMessage(sock, SUS_SM_ERROR, WSAGetLastError(), SUS_SOCKET_ERROR_FAILED_CLOSE);
		return FALSE;
	}
	sock->super = INVALID_SOCKET;
	sock->active = FALSE;
	susSocketCleanup(sock);
	SUS_PRINTDL("The connection is safely closed");
	return TRUE;
}

// -----------------------------------------------

//////////////////////////////////////////////////////////////////////////////////////////////////////
//									Low-level operations											//
//////////////////////////////////////////////////////////////////////////////////////////////////////

// -----------------------------------------------

// Define the address family
static SUS_SOCKET_ADDRESS SUSAPI susGetSocketAddressFamily(_In_ LPCSTR addr)
{
	SUS_ASSERT(addr);
	IN6_ADDR addr6 = { 0 };
	if (inet_pton(AF_INET6, addr, &addr6) == 1) return (SUS_SOCKET_ADDRESS) { .addr.Ipv6.sin6_family = AF_INET6, .addr.Ipv6.sin6_addr = addr6 };
	IN_ADDR addr4 = { 0 };
	if (inet_pton(AF_INET, addr, &addr4) == 1) return (SUS_SOCKET_ADDRESS) { .addr.Ipv4.sin_family = AF_INET, .addr.Ipv4.sin_addr = addr4 };
	return (SUS_SOCKET_ADDRESS) { 0 };
}
// Automatically create an address for the socket
SUS_SOCKET_ADDRESS susSocketAddress(_In_ LPCSTR addr, _In_ USHORT port) {
	SUS_ASSERT(addr);
	SUS_SOCKET_ADDRESS address = susGetSocketAddressFamily(addr);
	if (address.addr.Ipv4.sin_family == AF_UNSPEC) return address;
	if (address.addr.Ipv6.sin6_family == AF_INET6) address.addr.Ipv6.sin6_port = htons(port);
	else address.addr.Ipv4.sin_port = htons(port);
	return address;
}
// Get the socket address
SUS_SOCKET_ADDRESS SUSAPI susSocketGetAddress(_In_ SUS_LPSOCKET sock)
{
	SUS_ASSERT(sock && sock->super && sock->super != INVALID_SOCKET);
	SUS_SOCKET_ADDRESS peerAddr = { 0 };
	int peerAddrLen = sizeof(peerAddr);
	return getpeername(sock->super, (SOCKADDR*)&peerAddr, &peerAddrLen) == SOCKET_ERROR ? (SUS_SOCKET_ADDRESS) { 0 } : peerAddr;
}
// Get a static address string
LPCSTR SUSAPI susSocketAddressToString(_In_ SUS_SOCKET_ADDRESS address)
{
	SUS_ASSERT(address.addr.Ipv4.sin_family);
	static CHAR ipString[INET6_ADDRSTRLEN] = { 0 };
	inet_ntop(address.addr.Ipv4.sin_family, (LPVOID)(address.addr.Ipv6.sin6_family == AF_INET6 ? (LPVOID)&address.addr.Ipv6.sin6_addr : (LPVOID)&address.addr.Ipv4.sin_addr), ipString, sizeof(ipString));
	return ipString;
}

// -----------------------------------------------

// Binding an address to a socket
BOOL SUSAPI susSocketBind(_In_ SUS_LPSOCKET sock, _In_ USHORT port)
{
	SUS_PRINTDL("Binding an address to a socket");
	SUS_ASSERT(sock->super != INVALID_SOCKET);
	sock->address = susSocketAddress("0.0.0.0", port);
	if (bind(sock->super, (SOCKADDR*)&sock->address.addr, (int)(sock->address.addr.Ipv6.sin6_family == AF_INET6 ? sizeof(sock->address.addr.Ipv6) : sizeof(sock->address.addr.Ipv4))) == SOCKET_ERROR) {
		susSocketCallMessage(sock, SUS_SM_ERROR, WSAGetLastError(), SUS_SOCKET_ERROR_FAILED_BIND);
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
	SUS_ASSERT(sock->super != INVALID_SOCKET);
	if (listen(sock->super, SUS_SERVER_MAX_CLIENT_COUNT) == SOCKET_ERROR) {
		susSocketCallMessage(sock, SUS_SM_ERROR, WSAGetLastError(), SUS_SOCKET_ERROR_FAILED_LISTEN);
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
	SUS_ASSERT(server && server->super.super != INVALID_SOCKET && client);
	*client = susSocketSetup(server->clientHandler, server->super.userData);
	INT size = (int)(server->super.address.addr.Ipv6.sin6_family == AF_INET6 ? sizeof(server->super.address.addr.Ipv6) : sizeof(server->super.address.addr.Ipv4));
	client->super = accept(server->super.super, (SOCKADDR*)&client->address.addr, &size);
	if (client->super == INVALID_SOCKET) {
		SUS_PRINTDE("Couldn't accept connection");
		SUS_PRINTDC(WSAGetLastError());
		susSocketCallMessage(client, SUS_SM_ERROR, WSAGetLastError(), SUS_SOCKET_ERROR_FAILED_START);
		susSocketCleanup(client);
		return FALSE;
	}
	susSocketTunePerformance(client);
	susSocketSetNonBlocking(client, TRUE);
	client->active = TRUE;
	if (susSocketCallMessage(client, SUS_SM_CREATE, (WPARAM)server, (LPARAM)&client->address)) goto cleanup;
	return TRUE;
cleanup:
	SUS_PRINTDL("The user's CALLBACK decided to forcefully terminate the socket creation");
	susSocketShutdown(client);
	return FALSE;
}
// Connects to the server
BOOL SUSAPI susSocketConnect(_In_ SUS_LPSOCKET sock, _In_ LPCSTR addr, _In_ USHORT port)
{
	SUS_PRINTDL("Connecting to the server ...");
	SUS_ASSERT(sock && addr);
	sock->address = susSocketAddress(addr, port);
	if (!susBuildSocket(sock, sock->address.addr.si_family)) return FALSE;
	if (connect(sock->super, (SOCKADDR*)&sock->address.addr, (int)(sock->address.addr.Ipv6.sin6_family == AF_INET6 ? sizeof(sock->address.addr.Ipv6) : sizeof(sock->address.addr.Ipv4))) == SOCKET_ERROR) {
		SUS_PRINTDE("Couldn't connect to the server");
		SUS_PRINTDC(WSAGetLastError());
		susSocketCallMessage(sock, SUS_SM_ERROR, WSAGetLastError(), SUS_SOCKET_ERROR_FAILED_START);
		return FALSE;
	}
	susSocketTunePerformance(sock);
	susSocketSetNonBlocking(sock, TRUE);
	susSocketCallMessage(sock, SUS_SM_START, (WPARAM)0, (LPARAM)0);
	SUS_PRINTDL("Successful connection to the server");
	return TRUE;
}

// -----------------------------------------------

//////////////////////////////////////////////////////////////////////////////////////////////////////
//							Specific functions for working with sockets								//
//////////////////////////////////////////////////////////////////////////////////////////////////////

// -----------------------------------------------

// Set a timer for the socket
BOOL SUSAPI susSocketSetTimer(_Inout_ SUS_LPSOCKET sock, _In_ DWORD interval, _In_ DWORD id)
{
	SUS_PRINTDL("Setting a timer on a socket");
	SUS_ASSERT(sock && interval);
	if (!sock->timers && !(sock->timers = susNewMap(UINT, SUS_SOCKET_TIMER))) return FALSE;
	SUS_SOCKET_TIMER timer = { .interval = interval, .nextFire = GetTickCount64() + interval };
	return susMapSet(&sock->timers, &id, &timer) ? TRUE : FALSE;
}
// Delete the socket timer
VOID SUSAPI susSocketKillTimer(_Inout_ SUS_LPSOCKET sock, _In_ DWORD id)
{
	SUS_PRINTDL("Removing the timer from the socket");
	SUS_ASSERT(sock && sock->timers);
	susMapRemove(&sock->timers, &id);
}
// Running all suitable socket timers
static VOID SUSAPI susSocketTimersStart(_In_ SUS_LPSOCKET sock) {
	SUS_ASSERT(sock);
	if (!sock->handler || !sock->timers) return;
	ULONGLONG currTime = GetTickCount64();
	susMapForeach(sock->timers, i) {
		SUS_LPSOCKET_TIMER timer = susMapIterValue(i);
		if (currTime >= timer->nextFire) {
			sock->handler(sock, SUS_SM_TIMER, *(WPARAM*)susMapIterKey(i), timer->interval);
			timer->nextFire = currTime + timer->interval;
		}
	}
}

// -----------------------------------------------

// Set a property for a socket
BOOL SUSAPI susSocketSetProperty(_Inout_ SUS_LPSOCKET sock, _In_ LPCSTR key, _In_ SUS_USERDATA property)
{
	SUS_PRINTDL("Setting a new property - '%s'", key);
	SUS_ASSERT(sock && key);
	if (!sock->properties && !(sock->properties = susNewStringMap(SUS_USERDATA))) return FALSE;
	if (susMapContains(sock->properties, (SUS_OBJECT)&key)) return susMapSet(&sock->properties, &key, &property) ? TRUE : FALSE;
	key = sus_strdup(key);
	if (!susMapAdd(&sock->properties, &key, &property)) {
		SUS_PRINTDE("Couldn't create property");
		sus_strfree((LPSTR)key);
		return FALSE;
	}
	return TRUE;
}
// Get a property from a socket
SUS_USERDATA SUSAPI susSocketGetProperty(_In_ SUS_LPSOCKET sock, _In_ LPCSTR key)
{
	SUS_PRINTDL("Getting socket properties - '%s'", key);
	SUS_ASSERT(sock && key && sock->properties);
	if (!sock->properties) return 0;
	SUS_USERDATA* param = susMapGet(sock->properties, &key);
	return param ? *param : 0;
}

// -----------------------------------------------

//////////////////////////////////////////////////////////////////////////////////////////////////////
//											I/O operations											//
//////////////////////////////////////////////////////////////////////////////////////////////////////

// -----------------------------------------------

// Find a pair of null characters
static sus_lpbyte_t SUSAPI susFindDoubleNull(_In_ sus_lpbyte_t data, _In_ sus_size_t size) {
	SUS_ASSERT(data && size);
	for (; size > 1; data++, size--) if (!data[0] && !data[1]) return data;
	return NULL;
}

// -----------------------------------------------

// Get data from a socket
BOOL SUSAPI susSocketRead(_Inout_ SUS_LPSOCKET sock)
{
	SUS_ASSERT(sock && sock->super != INVALID_SOCKET && sock->buffers.readBuffer);
	sus_byte_t chunkBuffer[SUS_SOCKET_CHUNK_BUFFER_SIZE] = { 0 };
	INT bytesRead;
	do {
		bytesRead = recv(sock->super, (PCHAR)chunkBuffer, (INT)sizeof(chunkBuffer), 0);
		if (!bytesRead) {
			susSocketShutdown(sock);
			return FALSE;
		}
		if (bytesRead == SOCKET_ERROR) {
			INT err = WSAGetLastError();
			if (err == WSAEWOULDBLOCK) return TRUE;
			susSocketCallMessage(sock, SUS_SM_ERROR, (WPARAM)err, SUS_SOCKET_ERROR_FAILED_READ);
			return FALSE;
		}
		susBufferPush(&sock->buffers.readBuffer, chunkBuffer, bytesRead);
		do {
			sus_lpbyte_t endMsg = susFindDoubleNull(sock->buffers.readBuffer->data, sock->buffers.readBuffer->size);
			if (!endMsg) break;
			sus_size_t msgSize = (sus_size_t)(endMsg - sock->buffers.readBuffer->data);
			if (msgSize) susSocketCallMessage(sock, SUS_SM_DATA, (WPARAM)msgSize, (LPARAM)sock->buffers.readBuffer->data);
			susBufferErase(&sock->buffers.readBuffer, 0, (sus_size32_t)msgSize + 2);
		} while (sock->buffers.readBuffer->size);
		if (sock->buffers.readBuffer->size > SUS_SOCKET_MAX_MESSAGE_SIZE) if (sock->handler && !sock->handler(sock, SUS_SM_ERROR, (WPARAM)0, SUS_SOCKET_ERROR_BUFFER_OVERFLOW)) {
			susSocketEnd(sock);
			return FALSE;
		}
	} while (bytesRead == sizeof(chunkBuffer));
	return TRUE;
}
// Flushing the send buffer
sus_size_t SUSAPI susSocketFlush(_Inout_ SUS_LPSOCKET sock)
{
	SUS_ASSERT(sock && sock->super != INVALID_SOCKET && sock->buffers.writeBuffer);
	susSocketCallMessage(sock, SUS_SM_WRITE, 0, 0);
	while (sock->buffers.writeBuffer->size) {
		INT bytesWrite = send(sock->super, (PCHAR)sock->buffers.writeBuffer->data, (INT)sock->buffers.writeBuffer->size, 0);
		if (bytesWrite == SOCKET_ERROR) {
			INT err = WSAGetLastError();
			if (err == WSAEWOULDBLOCK) return sock->buffers.writeBuffer->size;
			susSocketCallMessage(sock, SUS_SM_ERROR, (WPARAM)err, (LPARAM)SUS_SOCKET_ERROR_FAILED_WRITE);
			return sock->buffers.writeBuffer->size;
		}
		susBufferErase(&sock->buffers.writeBuffer, 0, bytesWrite);
		SUS_PRINTDL("The socket sent %d bytes", bytesWrite);
	};
	return 0;
}

// -----------------------------------------------

//////////////////////////////////////////////////////////////////////////////////////////////////////
//										Working with the server										//
//////////////////////////////////////////////////////////////////////////////////////////////////////

// -----------------------------------------------

// Create a server
SUS_SERVER_SOCKET SUSAPI susServerSetup(_In_opt_ SUS_SOCKET_HANDLER serverHandler, _In_opt_ SUS_SOCKET_HANDLER clientHandler, _In_opt_ SUS_USERDATA userData)
{
	SUS_PRINTDL("Creating a server");
	SUS_SERVER_SOCKET server = { susSocketSetup(serverHandler, userData), .clientHandler = clientHandler };
	server.clientfds = susNewVector(sizeof(WSAPOLLFD));
	server.clients = susNewVector(sizeof(SUS_SOCKET));
	return server;
}
// Cleaning up server resources
static VOID SUSAPI susServerClose(_Inout_ SUS_LPSERVER_SOCKET server)
{
	SUS_ASSERT(server && server->clients && server->clientfds);
	susSocketClose((SUS_LPSOCKET)server);
	susVecForeach(i, server->clients) susSocketShutdown((SUS_LPSOCKET)susVectorAt(server->clients, i));
	susVectorDestroy(server->clientfds);
	susVectorDestroy(server->clients);
}
// Set the socket as a listening server
BOOL SUSAPI susServerListen(_Inout_ SUS_LPSERVER_SOCKET server, _In_ ADDRESS_FAMILY type, _In_ USHORT port)
{
	SUS_PRINTDL("Starting the server");
	if (!susBuildSocket(&server->super, type)) {
		SUS_PRINTDE("Couldn't start the server");
		return FALSE;
	}
	if (!susSocketBind(&server->super, port)) {
		SUS_PRINTDE("Couldn't start the server");
		closesocket(server->super.super);
		return FALSE;
	}
	if (!susSocketListen(&server->super)) {
		SUS_PRINTDE("Couldn't start the server");
		closesocket(server->super.super);
		return FALSE;
	}
	susSocketTunePerformance(&server->super);
	susSocketSetNonBlocking(&server->super, TRUE);
	SUS_PRINTDL("The server is running successfully! Server is listening for incoming connections on port: %d", port);
	return TRUE;
}

// -----------------------------------------------

//////////////////////////////////////////////////////////////////////////////////////////////////////
//										Socket Managers												//
//////////////////////////////////////////////////////////////////////////////////////////////////////

// -----------------------------------------------

// Add a client to the server list
static BOOL SUSAPI susServerAddClient(_Inout_ SUS_LPSERVER_SOCKET server, _In_ SUS_LPSOCKET client) {
	SUS_ASSERT(server && server->clientfds && server->clients && client);
	WSAPOLLFD fd = { .fd = client->super, .events = POLLIN | POLLOUT, .revents = 0 };
	if (!susVectorPush(&server->clientfds, &fd)) return FALSE;
	if (!susVectorPush(&server->clients, client)) return FALSE;
	susSocketCallMessage(client, SUS_SM_START, (WPARAM)0, (LPARAM)0);
	return TRUE;
}
// Accept connection to the stream
static BOOL SUSAPI susServerAcceptClient(_Inout_ SUS_LPSERVER_SOCKET server) {
	SUS_ASSERT(server);
	SUS_SOCKET client = { 0 };
	if (!susSocketAccept(server, &client)) goto error;
	if (!susServerAddClient(server, &client)) { susSocketCleanup(&client); goto error; }
	susSocketCallMessage((SUS_LPSOCKET)server, SUS_SM_START, (WPARAM)0, (LPARAM)&client);
	SUS_PRINTDL("Client %s connects to the server", susSocketAddressToString(client.address));
	return TRUE;
error:
	SUS_PRINTDE("Couldn't accept the client");
	return FALSE;
}
// Remove the client from the server list
static VOID SUSAPI susServerRemoveClient(_Inout_ SUS_LPSERVER_SOCKET server, _In_ UINT index) {
	SUS_ASSERT(server && server->clientfds && server->clients);
	susVectorErase(&server->clientfds, index);
	susVectorErase(&server->clients, index);
}
// Process a disconnected client
static VOID SUSAPI susServerClientError(_Inout_ SUS_LPSERVER_SOCKET server, _In_ UINT index) {
	SUS_ASSERT(server);
	SUS_LPSOCKET client = susVectorAt(server->clients, index);
	susSocketClose(client);
	susServerRemoveClient(server, index);
	SUS_PRINTDL("Client %s caused a critical error", susSocketAddressToString(client->address));
}
// Process a disconnected client
static VOID SUSAPI susServerClientClose(_Inout_ SUS_LPSERVER_SOCKET server, _In_ UINT index) {
	SUS_ASSERT(server);
	SUS_LPSOCKET client = susVectorAt(server->clients, index);
	susSocketShutdown(client);
	susServerRemoveClient(server, index);
	SUS_PRINTDL("Client %s has disconnected from the server", susSocketAddressToString(client->address));
}
// Process the client
static VOID SUSAPI susServerClientPoll(_In_ SUS_LPSERVER_SOCKET server, _In_ UINT index, _In_ SUS_LPSOCKET client, _In_ SHORT events)
{
	SUS_ASSERT(!(events & POLLNVAL));
	if (events & POLLERR) {
		INT error = susSocketGetError(client);
		if (error) {
			susSocketCallMessage(client, SUS_SM_ERROR, error, SUS_SOCKET_ERROR_POLL);
			susServerClientError(server, index);
			return;
		}
	}
	if (events & POLLHUP) {
		susSocketEnd(client);
	}
	if (events & POLLIN) {
		susSocketRead(client);
	}
	if (events & POLLOUT) {
		susSocketTimersStart(client);
		susSocketFlush(client);
	}
	if (!client->active) susServerClientClose(server, index);
}
// Survey of all clients of the stream
static BOOL SUSAPI susServerClientsUpdate(_In_ SUS_LPSERVER_SOCKET server)
{
	SUS_ASSERT(server && server->clientfds && server->clients);
	if (!server->clients->length) return TRUE;
	INT pollResult = WSAPoll((WSAPOLLFD*)server->clientfds->data, server->clientfds->length, SUS_SOCKET_POLL_TIMEOUT);
	if (pollResult < 0) return pollResult == 0;
	susVecForeachReverse(i, server->clients) {
		SUS_LPSOCKET client = susVectorAt(server->clients, i);
		LPWSAPOLLFD fd = susVectorAt(server->clientfds, i);
		if (!fd->revents) continue;
		susServerClientPoll(server, i, client, fd->revents);
	}
	return TRUE;
}
// Update the server status
BOOL SUSAPI susServerUpdate(_In_ SUS_LPSERVER_SOCKET server)
{
	SUS_ASSERT(server);
	susSocketTimersStart((SUS_LPSOCKET)server);
	WSAPOLLFD fd = { .fd = server->super.super, .events = POLLRDNORM, .revents = 0 };
	INT pollResult = WSAPoll(&fd, 1, SUS_SOCKET_POLL_TIMEOUT);
	if (!pollResult) goto fallback;
	if (pollResult == SOCKET_ERROR) goto error;
	if (fd.revents & POLLERR) {
		INT error = susSocketGetError((SUS_LPSOCKET)server);
		if (error) {
			susSocketCallMessage(&server->super, SUS_SM_ERROR, (WPARAM)error, SUS_SOCKET_ERROR_POLL);
			susServerClose(server);
			return FALSE;
		}
	}
	else if (fd.revents & POLLHUP) {
		susSocketEnd((SUS_LPSOCKET)server);
	}
	if (fd.revents & POLLRDNORM) {
		susServerAcceptClient(server);
	}
fallback:
	if (!server->super.active) { susServerClose(server); return FALSE; }
	Sleep(1);
	return susServerClientsUpdate(server);
error:
	SUS_PRINTDE("Failed to poll sockets");
	SUS_PRINTDC(WSAGetLastError());
	susSocketCallMessage(&server->super, SUS_SM_ERROR, (WPARAM)WSAGetLastError(), SUS_SOCKET_ERROR_FAILED_POLL);
	susServerClose(server);
	return FALSE;
}

// -----------------------------------------------

// Process the socket
BOOL SUSAPI susSocketUpdate(_In_ SUS_LPSOCKET sock)
{
	SUS_ASSERT(sock && sock->super != INVALID_SOCKET);
	WSAPOLLFD fd = { .fd = sock->super, .events = POLLIN | POLLOUT, .revents = 0 };
	INT pollResult = WSAPoll(&fd, 1, SUS_SOCKET_POLL_TIMEOUT);
	if (!pollResult) goto fallback;
	if (pollResult == SOCKET_ERROR) goto error;
	if (fd.revents & POLLERR) {
		INT error = susSocketGetError(sock);
		if (error) {
			susSocketCallMessage(sock, SUS_SM_ERROR, (WPARAM)error, SUS_SOCKET_ERROR_POLL);
			susSocketClose(sock);
			return FALSE;
		}
	}
	if (fd.revents & POLLHUP) {
		susSocketEnd(sock);
	}
	if (fd.revents & POLLIN) {
		susSocketRead(sock);
	}
	if (fd.revents & POLLOUT) {
		susSocketTimersStart(sock);
		susSocketFlush(sock);
	}
fallback:
	if (!sock->active) { susSocketShutdown(sock); return FALSE; }
	Sleep(1);
	return TRUE;
error:
	SUS_PRINTDE("Failed to poll sockets");
	SUS_PRINTDC(WSAGetLastError());
	susSocketCallMessage(sock, SUS_SM_ERROR, (WPARAM)WSAGetLastError(), SUS_SOCKET_ERROR_FAILED_POLL);
	susSocketClose(sock);
	return FALSE;
}

// -----------------------------------------------
