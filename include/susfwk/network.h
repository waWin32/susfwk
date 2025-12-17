// network.h
//
#ifndef _SUS_NETWORK_CORE_
#define _SUS_NETWORK_CORE_

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
	SUS_SM_START,	// Called after connecting to the host\param lParam: SUS_LPSOCKET_ADDRESS
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
	ULONGLONG nextFire;
} SUS_SOCKET_TIMER, *SUS_LPSOCKET_TIMER;

// Socket address
typedef union sus_socket_address { SOCKADDR_INET addr;  } SUS_SOCKET_ADDRESS;
// socket structure
typedef struct sus_socket {
	SOCKET				sock;		// System Socket Descriptor
	SUS_SOCKET_HANDLER	handler;	// Socket handler function
	SUS_BUFFER			readBuffer;	// Dynamic buffer for reading
	SUS_BUFFER			writeBuffer;// Dynamic buffer for writing
	SUS_HASHMAP			timers;		// UINT SUS_SOCKET_TIMER
	SUS_HASHMAP			properties;	// Additional properties of a socket with userData
	SUS_SOCKET_ADDRESS	address;	// Address socket
	SUS_OBJECT			userData;	// User data
} SUS_SOCKET, *SUS_LPSOCKET;
// The structure of the server socket
typedef struct sus_server_socket {
	SUS_SOCKET			_PARENT_;		// Parent
	SUS_VECTOR			clients;		// SUS_SOCKET
	SUS_VECTOR			clientfds;		// WSAPOLLFD
	SUS_SOCKET_HANDLER	heirHandler;	// Default client sockets callback
} SUS_SERVER_SOCKET, *SUS_LPSERVER_SOCKET;

// -------------------------------------------------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////////////////////////////////
//									Deinitializing the library										//
//////////////////////////////////////////////////////////////////////////////////////////////////////

// -----------------------------------------------

// Initializing the winsock
DWORD SUSAPI susNetworkSetup();
// Network closure
VOID SUSAPI susNetworkCleanup();

// -----------------------------------------------

//////////////////////////////////////////////////////////////////////////////////////////////////////
//									Basic socket operations											//
//////////////////////////////////////////////////////////////////////////////////////////////////////

// -----------------------------------------------

// Configure the base socket
SUS_SOCKET SUSAPI susSocketSetup(
	_In_opt_ SUS_SOCKET_HANDLER handler,
	_In_opt_ SUS_OBJECT userData
);
// Build a socket
BOOL SUSAPI susBuildSocket(
	_In_ SUS_LPSOCKET sock,
	_In_ ADDRESS_FAMILY type
);

// -----------------------------------------------

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

// -----------------------------------------------

//////////////////////////////////////////////////////////////////////////////////////////////////////
//							Specific functions for working with sockets								//
//////////////////////////////////////////////////////////////////////////////////////////////////////

// -----------------------------------------------

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

// -----------------------------------------------

// Set a property for a socket
BOOL SUSAPI susSocketSetProperty(
	_Inout_ SUS_LPSOCKET sock,
	_In_ LPCSTR key,
	_In_ LPARAM property
);
// Get a property from a socket
LPARAM SUSAPI susSocketGetProperty(
	_In_ SUS_LPSOCKET sock,
	_In_ LPCSTR key
);

// -----------------------------------------------

//////////////////////////////////////////////////////////////////////////////////////////////////////
//									Low-level operations											//
//////////////////////////////////////////////////////////////////////////////////////////////////////

// -----------------------------------------------

// Automatically create an address for the socket
SUS_SOCKET_ADDRESS susSocketAddress(
	_In_ LPCSTR addr,
	_In_ USHORT port
);
// Get the socket address
SUS_SOCKET_ADDRESS SUSAPI susSocketGetAddress(
	_In_ SUS_LPSOCKET sock
);
// Get a static address string
LPCSTR SUSAPI susSocketAddressToString(
	_In_ SUS_SOCKET_ADDRESS address
);

// -----------------------------------------------

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

// -----------------------------------------------

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

// -----------------------------------------------

// Check the availability of data to send
SUS_FORCEINLINE INT SUSAPI susSocketHasSendData(_Inout_ SUS_LPSOCKET sock)
{
	SUS_ASSERT(sock && sock->writeBuffer);
	return (INT)susBufferSize(sock->writeBuffer);
}
// Send data to the socket (_Null_terminated_)
SUS_FORCEINLINE BOOL SUSAPI susSocketWrite(_Inout_ SUS_LPSOCKET sock, _In_bytecount_(size) CONST LPBYTE data, _In_ SIZE_T size)
{
	SUS_PRINTDL("Socket write of %d bytes", size);
	SUS_ASSERT(sock && sock->writeBuffer && data && size && size < SUS_SOCKET_MAX_MESSAGE_SIZE);
	return susBufferAppend(&sock->writeBuffer, data, size) ? TRUE : FALSE;
}

// Send text to the socket
SUS_FORCEINLINE BOOL SUSAPI susSocketWriteText(_Inout_ SUS_LPSOCKET sock, _In_ LPCSTR text) {
	return susSocketWrite(sock, (LPBYTE)text, ((SIZE_T)lstrlenA(text) + 1) * sizeof(CHAR));
}
// Send text to the socket
SUS_FORCEINLINE BOOL SUSAPI susSocketWriteWText(_Inout_ SUS_LPSOCKET sock, _In_ LPCWSTR text) {
	return susSocketWrite(sock, (LPBYTE)text, ((SIZE_T)lstrlenW(text) + 1) * sizeof(WCHAR));
}

// -----------------------------------------------

//////////////////////////////////////////////////////////////////////////////////////////////////////
//									Fine-tuning the socket											//
//////////////////////////////////////////////////////////////////////////////////////////////////////

// -----------------------------------------------

// Set the socket as non-blocking
SUS_FORCEINLINE BOOL SUSAPI susSocketSetNonBlocking(_In_ SUS_LPSOCKET sock, BOOL enabled) {
	SUS_ASSERT(sock && sock->sock != INVALID_SOCKET);
	return !ioctlsocket(sock->sock, FIONBIO, &(u_long)enabled);
}
// Set the timeout for the socket
SUS_FORCEINLINE BOOL SUSAPI susSocketSetTimeout(_In_ SUS_LPSOCKET sock, DWORD timeout) {
	SUS_ASSERT(sock && sock->sock != INVALID_SOCKET);
	return !(setsockopt(sock->sock, SOL_SOCKET, SO_SNDTIMEO, (PCHAR)&timeout, sizeof(timeout)) || setsockopt(sock->sock, SOL_SOCKET, SO_RCVTIMEO, (PCHAR)&timeout, sizeof(timeout)));
}
// Set the timeout for the socket
SUS_FORCEINLINE DWORD SUSAPI susSocketGetReadData(_In_ SUS_LPSOCKET sock) {
	SUS_ASSERT(sock && sock->sock != INVALID_SOCKET);
	DWORD availableData = 0;
	return !ioctlsocket(sock->sock, FIONREAD, &availableData) ? availableData : (DWORD)-1;
}
// configuring socket performance
SUS_FORCEINLINE BOOL SUSAPI susSocketTunePerformance(_In_ SUS_LPSOCKET sock) {
	int buffSize = 1024 * 1024;
	setsockopt(sock->sock, SOL_SOCKET, SO_RCVBUF, (PCHAR)&buffSize, sizeof(buffSize));
	setsockopt(sock->sock, SOL_SOCKET, SO_SNDBUF, (PCHAR)&buffSize, sizeof(buffSize));
	int nodelay = 1;
	return setsockopt(sock->sock, IPPROTO_TCP, TCP_NODELAY, (PCHAR)&nodelay, sizeof(nodelay));
}
// Get Socket user data
SUS_FORCEINLINE SUS_OBJECT SUSAPI susSocketUserData(_In_ SUS_LPSOCKET sock) {
	SUS_ASSERT(sock);
	return sock->userData;
}
// Set Socket user data
SUS_FORCEINLINE VOID SUSAPI susSocketSetUserData(_Inout_ SUS_LPSOCKET sock, _In_ SUS_OBJECT userData) {
	SUS_ASSERT(sock);
	sock->userData = userData;
}
// Install the socket handler
SUS_FORCEINLINE VOID SUSAPI susSocketSetHandler(_In_ SUS_LPSOCKET sock, _In_ SUS_SOCKET_HANDLER handler) {
	SUS_ASSERT(sock);
	sock->handler = handler;
}
// Send a message to the socket
SUS_FORCEINLINE LRESULT SUSAPI susSocketCallMessage(_In_ SUS_LPSOCKET sock, _In_ SUS_SOCKET_MESSAGE uMsg, _In_ WPARAM wParam, _In_ LPARAM lParam) {
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
	_In_ ADDRESS_FAMILY type,
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
// Find the client in the server list
SUS_LPSOCKET SUSAPI susServerFindClient(
	_In_ SUS_LPSERVER_SOCKET server,
	_In_ LPCSTR ip
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
//										Additional functions									  //
////////////////////////////////////////////////////////////////////////////////////////////////////

// Format binary data into a string
LPSTR SUSAPI susBinToStr(_In_ SUS_DATAVIEW bin);
// Format a string into binary data
SUS_DATAVIEW SUSAPI susStrToBin(_In_ LPCSTR str);

//////////////////////////////////////////////////////////////////////////////////////////////////////
//											Code examples											//
//////////////////////////////////////////////////////////////////////////////////////////////////////

/* Example of the server code

LRESULT SUSAPI ClientHandler(SUS_LPSOCKET sock, SUS_SOCKET_MESSAGE uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case SUS_SM_START: {
		// Client processing after activation
	} return 0;
	case SUS_SM_DATA: {
		SUS_PRINTDL("Received data from the client:");
		sus_cwrite(GetStdHandle(STD_OUTPUT_HANDLE), lParam, wParam);
		sus_printf("\n");
	} return 0;
	case SUS_SM_END: {
		SUS_PRINTDL("the client has disconnected");
	} return 0;
	case SUS_SM_ERROR: {
		SUS_PRINTDE("Client error: %d", wParam);
	} return 0;
	default: return 0;
	}
}
LRESULT SUSAPI ServerHandler(SUS_LPSOCKET server, SUS_SOCKET_MESSAGE uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case SUS_SM_CREATE: {
		SUS_PRINTDL("The server is running, waiting for connections...");
	} return 0;
	case SUS_SM_START: {
		// When connecting a new client
		SUS_PRINTDL("%s join to server", susSocketAddressToString(*(SUS_SOCKET_ADDRESS*)lParam));
	} return 0;
	case SUS_SM_END: {
		SUS_PRINTDL("The server is closed");
	} return 0;
	case SUS_SM_ERROR: {
		SUS_PRINTDE("Server error: %d", wParam);
	} return 0;
	default: return 0;
	}
}

int main()
{
	SUS_CONSOLE_DEBUGGING();
	susNetworkSetup();
	SUS_SERVER_SOCKET server = susServerSetup(ServerHandler, ClientHandler, NULL);
	if (!susServerListen(&server, AF_INET, 8000)) return 1;
	while (susServerUpdate(&server));
	susNetworkCleanup();
	ExitProcess(0);
}

*/

#endif /* !_SUS_NETWORK_CORE_ */
