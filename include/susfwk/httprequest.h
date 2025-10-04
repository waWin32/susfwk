// httprequest.h
//
#ifndef _SUS_HTTP_REQUEST_
#define _SUS_HTTP_REQUEST_

#include <winhttp.h>
#pragma comment(lib, "winhttp.lib")

//////////////////////////////////////////////////////////////////////////////////////////
//								Basic http request structures							//
//////////////////////////////////////////////////////////////////////////////////////////

// ------------------------------------------------------------

// HTTP request structure
typedef struct sus_http_request {
	HINTERNET hConnect;	// Connection descriptor
	HINTERNET hRequest;	// Request descriptor
} SUS_HTTP_REQUEST, *SUS_LPHTTP_REQUEST;

// ------------------------------------------------------------

// HTTP response structure
typedef struct sus_http_response {
	DWORD		dwStatus;	// Numerical status from the server (0 - error)
	WCHAR		message[32];// Message from the server
	SYSTEMTIME	stTime;		// Response time
	SUS_HASHMAP headers;	// WCHAR[32] -> SUS_BUFFER
	struct sus_http_body {
		WCHAR		type[32];		// Content Type
		WCHAR		encoding[32];	// Content encryption
		SUS_BUFFER	buff;			// Content data buffer
	} body;	// Response body
} SUS_HTTP_RESPONSE, *SUS_LPHTTP_RESPONSE;

// ------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////////////////////
//						Functions for working with the http protocol					//
//////////////////////////////////////////////////////////////////////////////////////////

// ------------------------------------------------------------

// Creating an http session
BOOL SUSAPI susHttpSessionSetup(
	_In_opt_z_ LPCWSTR pszAgentW,
	_In_opt_ DWORD dwTimeOut
);
// Deleting an http session
VOID SUSAPI susHttpSessionCleanup();

// ------------------------------------------------------------

// Create a request
SUS_HTTP_REQUEST SUSAPI susHttpRequestSetup(
	_In_ LPCWSTR method,
	_In_ LPCWSTR lpUrl
);
// Add a header to a query
BOOL SUSAPI susHttpSetHeader(
	_Inout_ HINTERNET hRequest,
	_In_ LPCWSTR key,
	_In_ LPCWSTR value,
	_In_ BOOL replace
);
// Send a request
BOOL SUSAPI susHttpRequestExecute(
	_In_ HINTERNET hRequest,
	_In_opt_ LPCWSTR lpszHeaders,
	_In_opt_ SUS_DATAVIEW body
);
// Get a response from the server
SUS_HTTP_RESPONSE SUSAPI susHttpReceiveResponse(
	_In_ HINTERNET hRequest
);
// Request a request
SUS_HTTP_RESPONSE SUSAPI susHttpRequest(
	_In_ LPCWSTR method,
	_In_ LPCWSTR url,
	_In_opt_ LPCWSTR lpszHeaders,
	_In_opt_ SUS_DATAVIEW body
);

// ------------------------------------------------------------

// Close the http request
SUS_INLINE VOID SUSAPI susHttpRequestClose(_Inout_ SUS_LPHTTP_REQUEST req) {
	WinHttpCloseHandle(req->hRequest);
	req->hRequest = NULL;
	WinHttpCloseHandle(req->hConnect);
	req->hConnect = NULL;
}
// Clear the response data
SUS_INLINE VOID SUSAPI susHttpResponseCleanup(_Inout_ SUS_LPHTTP_RESPONSE res) {
	if (res->headers) {
		susMapForeach(res->headers, entry) {
			susBufferDestroy(*(SUS_BUFFER*)susMapValue(res->headers, entry));
		}
		susMapDestroy(res->headers);
	}
	res->headers = NULL;
	if (res->body.buff) susBufferDestroy(res->body.buff);
	res->body.buff = NULL;
}

// ------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////////////////////
//					Abstract functions for creating http requests						//
//////////////////////////////////////////////////////////////////////////////////////////

// ------------------------------------------------------------

// Send a GET request
SUS_INLINE SUS_HTTP_RESPONSE SUSAPI susHttpGet(_In_ LPCWSTR lpUrl, _In_opt_ LPCWSTR lpszHeaders) {
	return susHttpRequest(L"GET", lpUrl, lpszHeaders, (SUS_DATAVIEW) { 0 });
}
// Send a POST request
SUS_INLINE SUS_HTTP_RESPONSE SUSAPI susHttpPost(_In_ LPCWSTR lpUrl, _In_opt_ LPCWSTR lpszHeaders, _In_ SUS_DATAVIEW body) {
	return susHttpRequest(L"POST", lpUrl, lpszHeaders, body);
}
// Send a PUT request
SUS_INLINE SUS_HTTP_RESPONSE SUSAPI susHttpPut(_In_ LPCWSTR lpUrl, _In_opt_ LPCWSTR lpszHeaders, _In_opt_ SUS_DATAVIEW body) {
	return susHttpRequest(L"PUT", lpUrl, lpszHeaders, body);
}
// Send a DELETE request
SUS_INLINE SUS_HTTP_RESPONSE SUSAPI susHttpDelete(_In_ LPCWSTR lpUrl, _In_opt_ LPCWSTR lpszHeaders) {
	return susHttpRequest(L"DELETE", lpUrl, lpszHeaders, (SUS_DATAVIEW) { 0 });
}

// ------------------------------------------------------------

#endif // !_SUS_HTTP_REQUEST_
