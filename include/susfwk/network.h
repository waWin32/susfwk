// network.h
//
#ifndef _SUS_NETWORK_CORE_
#define _SUS_NETWORK_CORE_

#pragma comment(lib, "ws2_32.lib")
#pragma warning(disable: 4996)

// -------------------------------------------------------------------------------------------------------------

#define SUS_SOCKET_API_VERSION MAKEWORD(2, 2)
#define SUS_SOCKET_INIT_READ_BUFFER_SIZE 1024
#define SUS_SOCKET_INIT_WRITE_BUFFER_SIZE 1024
#define SUS_SOCKET_POLL_TIMEOUT 10
#define SUS_SOCKET_CHUNK_BUFFER_SIZE 512
#define SUS_SOCKET_MAX_MESSAGE_SIZE (SIZE_T)((1 << 20) * 20)

// Socket errors
typedef enum sus_socket_error {
	SUS_SOCKET_ERROR_UNKNOWN,			// Unknown error
	SUS_SOCKET_ERROR_FAILED_CREATE,		// Couldn't create socket
	SUS_SOCKET_ERROR_FAILED_BIND,		// Failed to bind an address to a socket
	SUS_SOCKET_ERROR_FAILED_LISTEN,		// Failed to start listening for incoming connections on the socket
	SUS_SOCKET_ERROR_FAILED_START,		// Failed to establish a connection between sockets
	SUS_SOCKET_ERROR_FAILED_CLOSE,		// Couldn't close connection
	SUS_SOCKET_ERROR_FAILED_READ,		// Couldn't read the data
	SUS_SOCKET_ERROR_BUFFER_OVERFLOW,	// The read buffer is full
	SUS_SOCKET_ERROR_FAILED_WRITE,		// Couldn't send data
	SUS_SOCKET_ERROR_FAILED_POLL,		// Failed to complete survey
	SUS_SOCKET_ERROR_POLL,				// Error in the socket polling process
	SUS_SOCKET_ERROR_USER				// User errors
} SUS_SOCKET_ERROR;

// -------------------------------------------------------------------------------------------------------------

// Socket Messages
typedef enum sus_socket_message {
	SUS_SM_UNKNOWN,	// Unknown
	SUS_SM_CREATE,	// Called after creating a socket
	SUS_SM_START,	// Called after connecting to the host
	SUS_SM_DATA,	// Called when data is received
	SUS_SM_WRITE,	// Called when you can send data
	SUS_SM_TIMER,	// It is called when the timer interval passes
	SUS_SM_ERROR,	// Called when an error occurs
	SUS_SM_END,		// Called after the connection is closed
	SUS_SM_USER		// User Messages
} SUS_SOCKET_MESSAGE;
// Socket processing function
typedef LRESULT(SUSAPI* SUS_SOCKET_HANDLER)(SUS_OBJECT sock, SUS_SOCKET_MESSAGE uMsg, WPARAM wParam, LPARAM lParam);
// Socket Timer
typedef struct sus_socket_timer {
	DWORD interval;
	DWORD nextFire;
} SUS_SOCKET_TIMER, *SUS_LPSOCKET_TIMER;
// socket structure
typedef struct sus_socket {
	SOCKET				sock;		// System Socket Descriptor
	SUS_SOCKET_HANDLER	handler;	// Socket handler function
	SUS_BUFFER			readBuffer;	// Dynamic buffer for reading
	SUS_BUFFER			writeBuffer;// Dynamic buffer for writing
	SUS_HASHMAP			timers;		// UINT SUS_SOCKET_TIMER
	SUS_OBJECT			userData;	// User data
	SOCKADDR_IN			addr;		// Address socket
} SUS_SOCKET, *SUS_LPSOCKET;
// The structure of the server socket
typedef struct sus_server_socket {
	SUS_SOCKET _PARENT_;
	SUS_VECTOR clients;					// SUS_SOCKET
	SUS_VECTOR clientfds;				// WSAPOLLFD
	SUS_SOCKET_HANDLER	heirHandler;	// Default client sockets callback
} SUS_SERVER_SOCKET, *SUS_LPSERVER_SOCKET;

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
SUS_SOCKET SUSAPI susSocketSetup(
	_In_opt_ SUS_SOCKET_HANDLER handler,
	_In_opt_ SUS_OBJECT userData
);
// Build a socket
BOOL SUSAPI susBuildSocket(
	_In_ SUS_LPSOCKET sock
);
// Closing a network socket
BOOL SUSAPI susSocketClose(
	_Inout_ SUS_LPSOCKET sock
);
// Standard connection closure
BOOL SUSAPI susSocketShutdown(
	_Inout_ SUS_LPSOCKET sock
);
// Clean the socket
VOID SUSAPI susSocketCleanup(
	_Inout_ SUS_LPSOCKET sock
);

//////////////////////////////////////////////////////////////////////////////////////////////////////
//									Working with a socket timer										//
//////////////////////////////////////////////////////////////////////////////////////////////////////

// Set a timer for the socket
BOOL SUSAPI susSocketSetTimer(
	_Inout_ SUS_LPSOCKET sock,
	_In_ DWORD interval,
	_In_ DWORD id
);
// Delete the socket timer
VOID SUSAPI susSocketKillTimer(
	_Inout_ SUS_LPSOCKET sock,
	_In_ DWORD id
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
	_In_ SUS_LPSOCKET sock,
	_In_ USHORT port
);
// Install Socket listening
BOOL SUSAPI susSocketListen(
	_In_ SUS_LPSOCKET sock
);
// Accept connection
BOOL SUSAPI susSocketAccept(
	_In_ SUS_LPSERVER_SOCKET server,
	_Out_ SUS_LPSOCKET client
);
// Connects to the server
BOOL SUSAPI susSocketConnect(
	_In_ SUS_LPSOCKET sock, 
	_In_ LPCSTR addr,
	_In_ USHORT port
);

//////////////////////////////////////////////////////////////////////////////////////////////////////
//											I/O operations											//
//////////////////////////////////////////////////////////////////////////////////////////////////////

// Get data from a socket
BOOL SUSAPI susSocketRead(
	_Inout_ SUS_LPSOCKET sock
);
// Flushing the send buffer
SIZE_T SUSAPI susSocketFlush(
	_Inout_ SUS_LPSOCKET sock
);
// Check the availability of data to send
SUS_INLINE INT SUSAPI susSocketHasSendData(_Inout_ SUS_LPSOCKET sock) {
	SUS_ASSERT(sock && sock->writeBuffer);
	return (INT)susBufferSize(sock->writeBuffer);
}
// Send data to the socket (_Null_terminated_)
SUS_INLINE VOID SUSAPI susSocketWrite(_Inout_ SUS_LPSOCKET sock, _In_bytecount_(size) CONST LPBYTE data, _In_ SIZE_T size) {
	SUS_ASSERT(sock && sock->writeBuffer && data && size && size < SUS_SOCKET_MAX_MESSAGE_SIZE);
	susBufferAppend(&sock->writeBuffer, data, size);
}

// Send text to the socket
SUS_INLINE VOID SUSAPI susSocketWriteText(_Inout_ SUS_LPSOCKET sock, _In_ LPCSTR text) {
	SUS_ASSERT(sock && text);
	susSocketWrite(sock, (LPBYTE)text, (SIZE_T)lstrlenA(text) + 1);
}
// Send text to the socket
SUS_INLINE VOID SUSAPI susSocketWriteWText(_Inout_ SUS_LPSOCKET sock, _In_ LPCWSTR text) {
	SUS_ASSERT(sock && text);
	susSocketWrite(sock, (LPBYTE)text, ((SIZE_T)lstrlenW(text) + 1) * sizeof(WCHAR));
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
//									Fine-tuning the socket											//
//////////////////////////////////////////////////////////////////////////////////////////////////////

// Set the socket as non-blocking
SUS_INLINE BOOL SUSAPI susSocketSetNonBlocking(_In_ SUS_LPSOCKET sock, BOOL enabled) {
	SUS_ASSERT(sock && sock->sock != INVALID_SOCKET);
	return !ioctlsocket(sock->sock, FIONBIO, &(u_long)enabled);
}
// Set the timeout for the socket
SUS_INLINE BOOL SUSAPI susSocketSetTimeout(_In_ SUS_LPSOCKET sock, DWORD timeout) {
	SUS_ASSERT(sock && sock->sock != INVALID_SOCKET);
	return !(setsockopt(sock->sock, SOL_SOCKET, SO_SNDTIMEO, (PCHAR)&timeout, sizeof(timeout)) || setsockopt(sock->sock, SOL_SOCKET, SO_RCVTIMEO, (PCHAR)&timeout, sizeof(timeout)));
}
// Set the timeout for the socket
SUS_INLINE DWORD SUSAPI susSocketGetReadData(_In_ SUS_LPSOCKET sock) {
	SUS_ASSERT(sock && sock->sock != INVALID_SOCKET);
	DWORD availableData = 0;
	return !ioctlsocket(sock->sock, FIONREAD, &availableData) ? availableData : (DWORD)-1;
}
// configuring socket performance
SUS_INLINE BOOL SUSAPI susSocketTunePerformance(_In_ SUS_LPSOCKET sock) {
	int buffSize = 1024 * 1024;
	setsockopt(sock->sock, SOL_SOCKET, SO_RCVBUF, (PCHAR)&buffSize, sizeof(buffSize));
	setsockopt(sock->sock, SOL_SOCKET, SO_SNDBUF, (PCHAR)&buffSize, sizeof(buffSize));
	int nodelay = 1;
	return setsockopt(sock->sock, IPPROTO_TCP, TCP_NODELAY, (PCHAR)&nodelay, sizeof(nodelay));
}
// Get Socket user data
SUS_INLINE SUS_OBJECT SUSAPI susSocketGetUserData(_In_ SUS_LPSOCKET sock) {
	SUS_ASSERT(sock);
	return sock->userData;
}
// Set Socket user data
SUS_INLINE VOID SUSAPI susSocketSetUserData(_Inout_ SUS_LPSOCKET sock, _In_ SUS_OBJECT userData) {
	SUS_ASSERT(sock);
	sock->userData = userData;
}
// Install the socket handler
SUS_INLINE VOID SUSAPI susSocketSetHandler(_In_ SUS_LPSOCKET sock, _In_ SUS_SOCKET_HANDLER handler) {
	SUS_ASSERT(sock);
	sock->handler = handler;
}
// Get the socket address
SUS_INLINE SOCKADDR_IN SUSAPI susSocketGetAddr(_In_ SUS_LPSOCKET sock) {
	SUS_ASSERT(sock);
	SOCKADDR_IN peerAddr = { 0 };
	int peerAddrLen = sizeof(peerAddr);
	return getpeername(sock->sock, (SOCKADDR*)&peerAddr, &peerAddrLen) ? (SOCKADDR_IN) { 0 } : peerAddr;
}
// Send a message to the socket
SUS_INLINE LRESULT SUSAPI susSocketSendMessage(_In_ SUS_LPSOCKET sock, _In_ SUS_SOCKET_MESSAGE uMsg, _In_ WPARAM wParam, _In_ LPARAM lParam) {
	SUS_ASSERT(sock && uMsg);
	if (sock->handler) return sock->handler(sock, uMsg, wParam, lParam);
	return 0;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
//										Working with the server										//
//////////////////////////////////////////////////////////////////////////////////////////////////////

// Create a server
SUS_SERVER_SOCKET SUSAPI susServerSetup(
	_In_opt_ SUS_SOCKET_HANDLER handler,
	_In_opt_ SUS_SOCKET_HANDLER heirHandler,
	_In_opt_ SUS_OBJECT userData
);
// Set the socket as a listening server
BOOL SUSAPI susServerListen(
	_Inout_ SUS_LPSERVER_SOCKET server,
	_In_ USHORT port
);
// Accepting the client to the server
SUS_LPSOCKET SUSAPI susServerAccept(
	_In_ SUS_LPSERVER_SOCKET server
);
// Cleaning up server resources
VOID SUSAPI susServerCleanup(
	_Inout_ SUS_LPSERVER_SOCKET server
);

//////////////////////////////////////////////////////////////////////////////////////////////////////
//										Socket Managers												//
//////////////////////////////////////////////////////////////////////////////////////////////////////

// Process the socket
BOOL SUSAPI susSocketUpdate(
	_In_ SUS_LPSOCKET sock
);
// Update the server status
BOOL SUSAPI susServerUpdate(
	_In_ SUS_LPSERVER_SOCKET server
);

////////////////////////////////////////////////////////////////////////////////////////////////////
//								      Formatting binary data									  //
////////////////////////////////////////////////////////////////////////////////////////////////////

// Format binary data into a string
LPSTR SUSAPI susBinToStr(_In_ SUS_DATAVIEW bin);
// Format a string into binary data
SUS_DATAVIEW SUSAPI susStrToBin(_In_ LPCSTR str);

//////////////////////////////////////////////////////////////////////////////////////////////////////
//											Code examples											//
//////////////////////////////////////////////////////////////////////////////////////////////////////

/* Example of the server code

BOOL SUSAPI ClientHandler(SUS_LPSOCKET sock, SUS_SOCKET_MESSAGE uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case SUS_SM_DATA: {
		SUS_PRINTDL("Received data from the client:");
		sus_cwrite(GetStdHandle(STD_OUTPUT_HANDLE), lParam, wParam);
		sus_printf("\n");
	} return TRUE;
	case SUS_SM_END: {
		SUS_PRINTDL("the client has disconnected");
	} return TRUE;
	case SUS_SM_ERROR: {
		SUS_PRINTDE("Client error: %d", wParam);
	} return TRUE;
	}
}
BOOL SUSAPI ServerHandler(SUS_LPSOCKET server, SUS_SOCKET_MESSAGE uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case SUS_SM_CREATE: {
		SUS_PRINTDL("The server is running, waiting for connections...");
	} return TRUE;
	case SUS_SM_START: {
		SUS_PRINTDL("%s join to server", inet_ntoa(((LPSOCKADDR_IN)wParam)->sin_addr));
	} return TRUE;
	case SUS_SM_END: {
		SUS_PRINTDL("The server is closed");
	} return TRUE;
	case SUS_SM_ERROR: {
		SUS_PRINTDE("Server error: %d", wParam);
	} return TRUE;
	}
}

int main()
{
	SUS_CONSOLE_DEBUGGING();
	susNetworkSetup();
	SUS_SERVER_SOCKET server = susServerSetup(ServerHandler, ClientHandler, NULL);
	susServerListen(&server, 8000);
	if (!susServerListen(&server, 8000)) return 1;
	while (susServerUpdate(&server));
	susNetworkCleanup();
	ExitProcess(0);
}

*/

#endif /* !_SUS_NETWORK_CORE_ */
