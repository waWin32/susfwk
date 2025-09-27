// httprequest.h
//
#ifndef _SUS_HTTP_REQUEST_
#define _SUS_HTTP_REQUEST_

#include <winhttp.h>

// HTTP request structure
typedef struct sus_http_request {
	HINTERNET hConnect;
	HINTERNET hRequest;
} SUS_HTTP_REQUEST, * SUS_LPHTTP_REQUEST;
// HTTP response structure
typedef struct sus_http_response {
	INT status;
	WCHAR message[32];
	SYSTEMTIME stReceived;
	SUS_HASHMAP headers; // WCHAR[32] -> WCHAR[64]
	SUS_BUFFER body;
} SUS_HTTP_RESPONSE, * SUS_LPHTTP_RESPONSE;

// configuring the http protocol
BOOL SUSAPI susHttpSetup();
// Clearing http protocol data
VOID SUSAPI susHttpCleanup();

// Create a request
BOOL SUSAPI susHttpRequestSetup(_In_ LPCWSTR method, _In_ LPCWSTR lpUrl, _Out_ SUS_LPHTTP_REQUEST req);
// Add a header to a query
BOOL SUSAPI susHttpAddHeader(_Inout_ HINTERNET hRequest, LPCWSTR key, LPCWSTR value);
// Send a request
BOOL SUSAPI susHttpSendRequest(_In_ HINTERNET hRequest, _In_opt_ SUS_DATAVIEW body);
// Get a response from the server
BOOL SUSAPI susHttpParseResponse(_In_ HINTERNET hRequest, _Out_ SUS_LPHTTP_RESPONSE response);
// Make a request
SUS_HTTP_RESPONSE SUSAPI susHttpPerformRequest(SUS_HTTP_REQUEST req, _In_opt_ SUS_DATAVIEW body);
// Clear the response data
SUS_INLINE VOID SUSAPI susHttpResponseCleanup(_Inout_ SUS_HTTP_RESPONSE res) {
	if (res.headers) susMapDestroy(res.headers);
	if (res.body) susBufferDestroy(res.body);
}

#pragma comment(lib, "winhttp.lib")

#endif // !_SUS_HTTP_REQUEST_
