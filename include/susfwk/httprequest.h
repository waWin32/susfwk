// httprequest.h
//
#ifndef _SUS_HTTP_REQUEST_
#define _SUS_HTTP_REQUEST_

#if defined(_WIN32) || defined(_WIN64)
#pragma comment(lib, "winhttp.lib")
#endif // !_WIN32 _WIN64

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

// Standard http body content data types
typedef enum sus_http_content_type {
	SUS_HTTP_CONTENT_TYPE_TEXT,			// text/plain
	SUS_HTTP_CONTENT_TYPE_HTTP,			// text/http
	SUS_HTTP_CONTENT_TYPE_CSS,			// text/css
	SUS_HTTP_CONTENT_TYPE_IMAGE,		// image/jpeg
	SUS_HTTP_CONTENT_TYPE_AUDIO,		// audio/mpeg
	SUS_HTTP_CONTENT_TYPE_JAVASCRIPT,	// application/javascript
	SUS_HTTP_CONTENT_TYPE_JSON,			// application/json
	SUS_HTTP_CONTENT_TYPE_JSON_ERROR,	// application/problem+json
	SUS_HTTP_CONTENT_TYPE_PDF			// application/pdf
} SUS_HTTP_CONTENT_TYPE, *SUS_LPHTTP_CONTENT_TYPE;
// The type of encryption for http content data
typedef enum sus_http_content_encoding {
	SUS_HTTP_CONTENT_ENCODING_DEFAULT,	// identity
	SUS_HTTP_CONTENT_ENCODING_GZIP,		// gzip
	SUS_HTTP_CONTENT_ENCODING_DEFLATE,	// deflate
	SUS_HTTP_CONTENT_ENCODING_BROTLI	// br
} SUS_HTTP_CONTENT_ENCODING, *SUS_LPHTTP_CONTENT_ENCODING;
// Message body
typedef struct sus_http_body {
	SUS_HTTP_CONTENT_TYPE		type;		// Message Content Type
	SUS_HTTP_CONTENT_ENCODING	encoding;	// The type of encoding of the message content
	SUS_DATAVIEW				content;	// Content of the message body
} SUS_HTTP_BODY, *SUS_LPHTTP_BODY;
// HTTP response structure
typedef struct sus_http_response {
	DWORD		dwStatus;	// Numerical status from the server (0 - error)
	WCHAR		message[32];// Message from the server
	SYSTEMTIME	stTime;		// Response time
	SUS_HASHMAP headers;	// WCHAR[32] -> SUS_BUFFER
	SUS_HTTP_BODY body;		// The body received from the server
} SUS_HTTP_RESPONSE, *SUS_LPHTTP_RESPONSE;

// ------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////////////////////
//						Functions for working with the http protocol					//
//////////////////////////////////////////////////////////////////////////////////////////

// ------------------------------------------------------------

// Creating an http session
BOOL SUSAPI susHttpSessionSetup(
	_In_opt_z_ LPCWSTR pszAgentW,
	_In_opt_ DWORD dwTimeOut,
	_In_ BOOL useHttp2			// Windows 10+
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
VOID SUSAPI susHttpRequestClose(
	_Inout_ SUS_LPHTTP_REQUEST req
);
// Clear the response data
VOID SUSAPI susHttpResponseCleanup(
	_Inout_ SUS_LPHTTP_RESPONSE res
);

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
// Send a PATCH request
SUS_INLINE SUS_HTTP_RESPONSE SUSAPI susHttpPatch(_In_ LPCWSTR lpUrl, _In_opt_ LPCWSTR lpszHeaders, _In_opt_ SUS_DATAVIEW body) {
	return susHttpRequest(L"PATCH", lpUrl, lpszHeaders, body);
}

// ------------------------------------------------------------

#endif // !_SUS_HTTP_REQUEST_
