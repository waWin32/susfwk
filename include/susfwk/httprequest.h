// httprequest.h
//
#ifndef _SUS_HTTP_REQUEST_
#define _SUS_HTTP_REQUEST_

#include <winhttp.h>

//
typedef struct sus_http_request {
	HINTERNET hConnect;
	HINTERNET hRequest;
} SUS_HTTP_REQUEST, * SUS_LPHTTP_REQUEST;
//
typedef struct sus_http_url {
	INTERNET_PORT port;
	WCHAR host[64];
	WCHAR path[MAX_PATH];
} SUS_HTTP_URL, * SUS_LPHTTP_URL;
//
typedef struct sus_http_response {
	INT status;
	WCHAR message[32];
	SYSTEMTIME stReceived;
	SUS_HASHMAP headers; // WCHAR[32] -> WCHAR[64]
	SUS_DATAVIEW body;
} SUS_HTTP_RESPONSE, * SUS_LPHTTP_RESPONSE;

// configuring the http protocol
BOOL SUSAPI susHttpSetup();
// Clearing http protocol data
VOID SUSAPI susHttpCleanup();

// 
SUS_HTTP_REQUEST SUSAPI susHttpRequestSetup(_In_ LPCWSTR method, _In_ LPCWSTR lpUrl);
// 
BOOL SUSAPI susHttpSendRequest(_In_ HINTERNET hRequest, _In_opt_ SUS_DATAVIEW body);
// 
BOOL SUSAPI susHttpAddHeader(_Inout_ HINTERNET hRequest, LPCWSTR key, LPCWSTR value);
// 
SUS_HTTP_RESPONSE SUSAPI susHttpGetResponse(_In_ HINTERNET hRequest);
//
SUS_HTTP_RESPONSE SUSAPI susHttpPerformRequest(SUS_HTTP_REQUEST req, _In_opt_ SUS_DATAVIEW body);
//
SUS_INLINE VOID SUSAPI susHttpResponseCleanup(_Inout_ SUS_HTTP_RESPONSE res) {
	if (res.headers) susMapDestroy(res.headers);
	if (res.body.data) susDataDestroy(res.body);
}

#pragma comment(lib, "winhttp.lib")

#endif // !_SUS_HTTP_REQUEST_
