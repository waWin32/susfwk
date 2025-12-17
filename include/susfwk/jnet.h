// jnet.h
//
#ifndef _SUS_JNET_
#define _SUS_JNET_

#include "json.h"

// ================================================================================================

// -----------------------------------------------

// JNET property name
#define SUS_JNET_PROPERTY "/jnet/"

// -----------------------------------------------

// JNET query methods
typedef enum sus_jnet_method {
	SUS_JNET_METHOD_UNKNOWN,
	SUS_JNET_METHOD_GET,
	SUS_JNET_METHOD_POST,
	SUS_JNET_METHOD_PUT,
	SUS_JNET_METHOD_PATCH,
	SUS_JNET_METHOD_DELETE,
	SUS_JNET_METHOD_COMMAND,
	SUS_JNET_METHOD_SUBSCRIBE,
	SUS_JNET_METHOD_UNSUBSCRIBE,
	SUS_JNET_METHOD_PING,
	SUS_JNET_METHOD_ERROR
} SUS_JNET_METHOD;

// -----------------------------------------------

// JNET response handler
typedef VOID(SUSAPI* SUS_JNET_RESPONSE_HANDLER)(_In_ SUS_LPSOCKET sock, _In_ SUS_LPJSON id, _In_ INT status, _In_ SUS_LPJSON headers, _In_ SUS_LPJSON body);
// JNET request handler
typedef SUS_JSON(SUSAPI* SUS_JNET_REQUEST_HANDLER)(_In_ SUS_LPSOCKET sock, _In_ SUS_LPJSON id, _In_ SUS_JNET_METHOD method, _In_ LPCSTR path, _In_ SUS_LPJSON headers, _In_ SUS_LPJSON body);
// JNET notification handler
typedef VOID(SUSAPI* SUS_JNET_NOTIFICATION_HANDLER)(_In_ SUS_LPSOCKET sock, _In_ LPCSTR path, _In_ SUS_LPJSON headers, _In_ SUS_LPJSON body);

// -----------------------------------------------

// Addition to SUS_SOCKET
typedef struct sus_jnet {
	SUS_JNET_RESPONSE_HANDLER		resHandler;
	SUS_JNET_REQUEST_HANDLER		reqHandler;
	SUS_JNET_NOTIFICATION_HANDLER	msgHandler;
} SUS_JNET_STRUCT, *SUS_JNET;

// -----------------------------------------------

// ================================================================================================

// -----------------------------------------------

// Create a new JNET object
SUS_JNET SUSAPI susNewJnet();
// Delete a JNET object
VOID SUSAPI susJnetDestroy(
	_In_ SUS_JNET jnet
);

// -----------------------------------------------

// Send json to the socket
BOOL SUSAPI susJnetSend(
	_Inout_ SUS_LPSOCKET sock,
	_In_ SUS_JSON json
);
// JNET Socket Message Handler
LRESULT SUSAPI susJnetSocketHandler(
	_In_ SUS_LPSOCKET sock,
	_In_ SUS_SOCKET_MESSAGE uMsg,
	_In_ WPARAM wParam,
	_In_ LPARAM lParam
);

// -----------------------------------------------

// ================================================================================================

// -----------------------------------------------

// Create a JNET request
SUS_JSON SUSAPI susJnetRequestSetup(
	_In_ SUS_JSON id,
	_In_ SUS_JNET_METHOD method,
	_In_opt_ LPCSTR path,
	_In_opt_ SUS_JSON headers,
	_In_opt_ SUS_JSON body
);
// Send a request
SUS_INLINE BOOL SUSAPI susJnetRequest(_Inout_ SUS_LPSOCKET sock, _In_ SUS_JSON id, _In_ SUS_JNET_METHOD method, _In_opt_ LPCSTR path, _In_opt_ SUS_JSON headers, _In_opt_ SUS_JSON body) {
	SUS_JSON json = susJnetRequestSetup(id, method, path, headers, body);
	BOOL res = susJnetSend(sock, json);
	susJsonDestroy(&json);
	return res;
}

// -----------------------------------------------

// Create a JNET message
SUS_JSON SUSAPI susJnetNotificationSetup(
	_In_opt_ LPCSTR path,
	_In_opt_ SUS_JSON headers,
	_In_opt_ SUS_JSON body
);
// Send a message
SUS_INLINE BOOL SUSAPI susJnetNotification(_Inout_ SUS_LPSOCKET sock, _In_opt_ LPCSTR path, _In_opt_ SUS_JSON headers, _In_opt_ SUS_JSON body) {
	SUS_JSON json = susJnetNotificationSetup(path, headers, body);
	BOOL res = susJnetSend(sock, json);
	susJsonDestroy(&json);
	return res;
}

// -----------------------------------------------

// Create a JNET response
SUS_JSON SUSAPI susJnetResponseSetup(
	_In_ SUS_JSON id,
	_In_ INT status,
	_In_opt_ SUS_JSON headers,
	_In_opt_ SUS_JSON body
);
// Send a response
SUS_INLINE BOOL SUSAPI susJnetResponse(_Inout_ SUS_LPSOCKET sock, _In_ SUS_JSON id, _In_ INT status, _In_opt_ SUS_JSON headers, _In_opt_ SUS_JSON body) {
	SUS_JSON json = susJnetResponseSetup(id, status, headers, body);
	BOOL res = susJnetSend(sock, json);
	susJsonDestroy(&json);
	return res;
}

// -----------------------------------------------

// Install handlers for the jnet socket
VOID SUSAPI susJnetSetup(
	_Inout_ SUS_LPSOCKET sock,
	_In_opt_ SUS_JNET_RESPONSE_HANDLER resHandler,
	_In_opt_ SUS_JNET_REQUEST_HANDLER reqHandler,
	_In_opt_ SUS_JNET_NOTIFICATION_HANDLER msgHandler
);
// Get auto id for json request
SUS_INLINE SUS_JSON SUSAPI susJnetAutoId() {
	static INT id = 1;
	return susJsonNumber((sus_f32)id++);
}

// -----------------------------------------------

// ================================================================================================

#endif /* !_SUS_JNET_ */