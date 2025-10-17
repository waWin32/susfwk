// httprequest.c
//
#include "coreframe.h"
#include "include/susfwk/core.h"
#include "include/susfwk/memory.h"
#include "include/susfwk/vector.h"
#include "include/susfwk/hashtable.h"
#include "include/susfwk/httprequest.h"

// Global session descriptor
SUS_STATIC HINTERNET hSession = NULL;

//////////////////////////////////////////////////////////////////////////////////////////
//					Functions for formatting and receiving a response					//
//////////////////////////////////////////////////////////////////////////////////////////

// ------------------------------------------------------------

// URL structure
typedef struct sus_http_url {
	INTERNET_PORT port;
	WCHAR host[64];
	WCHAR path[MAX_PATH];
} SUS_HTTP_URL, * SUS_LPHTTP_URL;

// Parse the URL
static BOOL SUSAPI susHttpParseUrl(_In_ LPCWSTR textUrl, _Out_ SUS_LPHTTP_URL url)
{
	SUS_PRINTDL("Parsing the URL of an http request");
	SUS_ASSERT(textUrl && *textUrl && url && lstrlenW(textUrl) < 512);
	*url = (SUS_HTTP_URL){ 0 };
	URL_COMPONENTS uc = { 0 };
	uc.dwStructSize = sizeof(uc);
	uc.dwHostNameLength = uc.dwUrlPathLength = uc.dwExtraInfoLength = 1;
	if (!WinHttpCrackUrl(textUrl, 0, 0, &uc)) {
		SUS_PRINTDE("Request url could not be parsed");
		SUS_PRINTDC(GetLastError());
		return FALSE;
	}
	if (uc.lpszHostName) {
		SIZE_T maxChars = sizeof(url->host) / sizeof(WCHAR) - 1;
		SIZE_T len = min(maxChars, uc.dwHostNameLength);
		sus_memcpy((LPBYTE)url->host, (LPBYTE)uc.lpszHostName, len * sizeof(WCHAR));
		url->host[len] = L'\0';
	}
	else url->host[0] = L'\0';
	if (uc.lpszUrlPath) {
		SIZE_T maxChars = sizeof(url->path) / sizeof(WCHAR) - 1;
		SIZE_T len = min(maxChars, uc.dwUrlPathLength);
		sus_memcpy((LPBYTE)url->path, (LPBYTE)uc.lpszUrlPath, len * sizeof(WCHAR));
		if (uc.lpszExtraInfo) {
			SIZE_T remaining = maxChars - len;
			SIZE_T exlen = min(uc.dwExtraInfoLength, remaining);
			sus_memcpy((LPBYTE)url->path + len * sizeof(WCHAR), (LPBYTE)uc.lpszExtraInfo, exlen * sizeof(WCHAR));
			url->path[len + exlen] = L'\0';
		}
		else url->path[len] = L'\0';
	}
	else url->path[0] = L'\0';
	url->port = uc.nPort;
	SUS_PRINTDL("The request URL has been successfully parsed");
	return TRUE;
}

// Process the header line
static BOOL SUSAPI susHttpParseHeaderLine(_Inout_ LPWSTR* pwszHeaders, _Inout_ SUS_LPHASHMAP map) {
	SUS_ASSERT(map && *map && pwszHeaders);
	if (!*pwszHeaders) return FALSE;
	LPWSTR key = sus_strtokW(pwszHeaders, L": ");
	if (!key || lstrlenW(key) >= 32) {
		return FALSE;
	}
	WCHAR keyBuff[32] = { 0 };
	lstrcpyW(keyBuff, key);
	LPWSTR value = sus_strtokW(pwszHeaders, L"\r\n");
	if (!value) return FALSE;
	SUS_LPBUFFER lpValueBuff = (SUS_BUFFER*)susMapGet(*map, keyBuff);
	if (!lpValueBuff) {
		SUS_BUFFER buff = susNewBuffer((lstrlenW(value) + 1) * sizeof(WCHAR));
		lpValueBuff = susMapAdd(map, keyBuff, &buff);
		if (!lpValueBuff) {
			susBufferDestroy(buff);
			return FALSE;
		}
	}
	else susBufferAppend(lpValueBuff, (LPBYTE)L"; ", 4);
	return susBufferAppend(lpValueBuff, (LPBYTE)value, lstrlenW(value) * sizeof(WCHAR)) != NULL;
}

// ------------------------------------------------------------

// Request a response status code
static VOID SUSAPI susHttpQueryStatusCode(_In_ HINTERNET hRequest, _Out_ LPDWORD pdwStatus) {
	SUS_ASSERT(hRequest && pdwStatus);
	DWORD dwStatusSize = sizeof(*pdwStatus);
	WinHttpQueryHeaders(hRequest,
		WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER,
		WINHTTP_HEADER_NAME_BY_INDEX, pdwStatus,
		&dwStatusSize, WINHTTP_NO_HEADER_INDEX
	);
}
// Request a status message
static VOID SUSAPI susHttpQueryStatusMessage(_In_ HINTERNET hRequest, _Out_ LPWSTR msg, _In_ DWORD size) {
	SUS_ASSERT(hRequest && msg && size);
	*msg = L'\0';
	WinHttpQueryHeaders(hRequest,
		WINHTTP_QUERY_STATUS_TEXT,
		WINHTTP_HEADER_NAME_BY_INDEX, msg,
		&size, WINHTTP_NO_HEADER_INDEX
	);
	*((LPWSTR)((LPBYTE)msg + size)) = L'\0';
}
// Request the time of the request
static VOID SUSAPI susHttpQueryTime(_In_ HINTERNET hRequest, _Out_ LPSYSTEMTIME time) {
	SUS_ASSERT(hRequest && time);
	DWORD dwTimeSize = sizeof(*time);
	WinHttpQueryHeaders(hRequest,
		WINHTTP_QUERY_DATE | WINHTTP_QUERY_FLAG_SYSTEMTIME,
		WINHTTP_HEADER_NAME_BY_INDEX, time,
		&dwTimeSize, WINHTTP_NO_HEADER_INDEX
	);
}
// Get the content type
static SUS_HTTP_CONTENT_TYPE SUSAPI susHttpQueryContentType(_In_ HINTERNET hRequest) {
	SUS_ASSERT(hRequest);
	WCHAR buff[32] = { 0 };
	DWORD size = sizeof(buff);
	if (!WinHttpQueryHeaders(hRequest,
		WINHTTP_QUERY_CONTENT_TYPE,
		WINHTTP_HEADER_NAME_BY_INDEX, buff,
		&size, WINHTTP_NO_HEADER_INDEX)
	) {
		SUS_PRINTDW("Couldn't get the content type");
		return SUS_HTTP_CONTENT_TYPE_TEXT;
	}
	*((LPWSTR)((LPBYTE)buff + size)) = L'\0';
	static LPCWSTR templates[] = { L"text/plain", L"text/http", L"text/css", L"image/jpeg", L"audio/mpeg", L"application/javascript", L"application/json", L"application/problem+json", L"application/pdf" };
	for (DWORD i = 0; i < SUS_COUNT_OF(templates); i++) if (lstrcmpiW(buff, templates[i]) == 0) return (SUS_HTTP_CONTENT_TYPE)i;
	return SUS_HTTP_CONTENT_TYPE_TEXT;
}
// Get content encryption
static SUS_HTTP_CONTENT_ENCODING SUSAPI susHttpQueryContentEncoding(_In_ HINTERNET hRequest) {
	SUS_ASSERT(hRequest);
	WCHAR buff[32] = { 0 };
	DWORD size = sizeof(buff);
	if (!WinHttpQueryHeaders(hRequest,
		WINHTTP_QUERY_CONTENT_ENCODING,
		WINHTTP_HEADER_NAME_BY_INDEX, buff,
		&size, WINHTTP_NO_HEADER_INDEX)
	) {
		SUS_PRINTDW("Couldn't get the encryption type");
		return SUS_HTTP_CONTENT_ENCODING_DEFAULT;
	}
	*((LPWSTR)((LPBYTE)buff + size)) = L'\0';
	static LPCWSTR templates[] = { L"identity", L"gzip", L"deflate", L"br" };
	for (DWORD i = 0; i < SUS_COUNT_OF(templates); i++) if (lstrcmpiW(buff, templates[i]) == 0) return (SUS_HTTP_CONTENT_ENCODING)i;
	return SUS_HTTP_CONTENT_ENCODING_DEFAULT;
}
// Request response headers
static BOOL SUSAPI susHttpQueryHeaders(_In_ HINTERNET hRequest, _Inout_ SUS_LPHASHMAP headers) {
	SUS_ASSERT(hRequest && headers);
	SUS_PRINTDL("HTTP header processing");
	DWORD dwHeaderSize = 0;
	*headers = susNewMap(WCHAR[32], SUS_BUFFER);
	if (!WinHttpQueryHeaders(hRequest,
		WINHTTP_QUERY_RAW_HEADERS_CRLF,
		WINHTTP_HEADER_NAME_BY_INDEX,
		WINHTTP_NO_OUTPUT_BUFFER,
		&dwHeaderSize, WINHTTP_NO_HEADER_INDEX
	) && GetLastError() == ERROR_INSUFFICIENT_BUFFER) {
		LPWSTR pwszHeaders = sus_calloc(dwHeaderSize + 1, sizeof(WCHAR));
		WinHttpQueryHeaders(hRequest,
			WINHTTP_QUERY_RAW_HEADERS_CRLF, WINHTTP_HEADER_NAME_BY_INDEX,
			pwszHeaders, &dwHeaderSize, WINHTTP_NO_HEADER_INDEX
		);
		LPWSTR ctx = pwszHeaders;
		sus_strtokW(&ctx, L"\r\n");
		while (susHttpParseHeaderLine(&ctx, headers));
		sus_free(pwszHeaders);
		return TRUE;
	}
	SUS_PRINTDE("Couldn't get the headers");
	SUS_PRINTDC(GetLastError());
	return FALSE;
}
// Request a response body
static BOOL SUSAPI susHttpQueryBody(_In_ HINTERNET hRequest, _Out_ SUS_LPDATAVIEW body) {
	SUS_ASSERT(hRequest && body);
	*body = (SUS_DATAVIEW){ 0 };
	SUS_BUFFER buff = susNewBuffer(1024);
	DWORD dwAvailable = 0;
	while (WinHttpQueryDataAvailable(hRequest, &dwAvailable) && dwAvailable) {
		DWORD read = 0;
		if (!WinHttpReadData(hRequest, susBufferAppend(&buff, NULL, dwAvailable), dwAvailable, &read) || !read) {
			susBufferDestroy(buff);
			SUS_PRINTDE("Couldn't read response body data");
			return FALSE;
		}
	}
	*body = susNewData(buff->size);
	sus_memcpy(body->data, buff->data, buff->size);
	susBufferDestroy(buff);
	return TRUE;
}

// ------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////////////////////
//						Functions for working with the http protocol					//
//////////////////////////////////////////////////////////////////////////////////////////

// ------------------------------------------------------------

// Creating an http session
BOOL SUSAPI susHttpSessionSetup(_In_opt_z_ LPCWSTR pszAgentW, _In_opt_ DWORD dwTimeOut, _In_ BOOL useHttp2)
{
	SUS_PRINTDL("Opening of the http session");
	SUS_ASSERT(!hSession);
	hSession = WinHttpOpen(
		pszAgentW ? pszAgentW : L"SUSAgent",
		WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
		WINHTTP_NO_PROXY_NAME,
		WINHTTP_NO_PROXY_BYPASS,
		0
	);
	if (!hSession) {
		SUS_PRINTDE("Failed to initialize the session");
		SUS_PRINTDC(GetLastError());
		return FALSE;
	}
	if (dwTimeOut) {
		SUS_PRINTDL("Setting the timeout");
		if (!WinHttpSetTimeouts(hSession, dwTimeOut, dwTimeOut, dwTimeOut, dwTimeOut)) {
			WinHttpCloseHandle(hSession);
			SUS_PRINTDE("The timeout could not be set");
			SUS_PRINTDC(GetLastError());
		}
	}
	if (useHttp2) {
		SUS_PRINTDL("Installing HTTP/2");
		DWORD http2 = WINHTTP_PROTOCOL_FLAG_HTTP2;
		if (!WinHttpSetOption(hSession, WINHTTP_OPTION_ENABLE_HTTP_PROTOCOL, &http2, (DWORD)sizeof(http2))) {
			SUS_PRINTDE("Couldn't install HTTP/2 protocol");
			SUS_PRINTDC(GetLastError());
			return FALSE;
		}
	}
	return TRUE;
}

// Deleting an http session
VOID SUSAPI susHttpSessionCleanup()
{
	SUS_PRINTDL("Closing the http session");
	SUS_ASSERT(hSession);
	WinHttpCloseHandle(hSession);
	hSession = NULL;
}

// ------------------------------------------------------------

// Create a request
SUS_HTTP_REQUEST SUSAPI susHttpRequestSetup(_In_ LPCWSTR method, _In_ LPCWSTR lpUrl)
{
	SUS_PRINTDL("Creating a request");
	SUS_ASSERT(method && lpUrl);
	SUS_HTTP_REQUEST req = { 0 };
	SUS_HTTP_URL url = { 0 };
	if (!susHttpParseUrl(lpUrl, &url)) {
		SUS_PRINTDE("Failed to create a request");
		return (SUS_HTTP_REQUEST) { 0 };
	}
	req.hConnect = WinHttpConnect(hSession, url.host, url.port, 0);
	if (!req.hConnect) {
		SUS_PRINTDE("Failed to create a request");
		return (SUS_HTTP_REQUEST) { 0 };
	}
	req.hRequest = WinHttpOpenRequest(
		req.hConnect, method, url.path, NULL,
		WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES,
		(url.port == INTERNET_DEFAULT_HTTPS_PORT) ? WINHTTP_FLAG_SECURE : 0
	);
	if (!req.hRequest) {
		WinHttpCloseHandle(req.hConnect);
		SUS_PRINTDE("Failed to create a request");
		return (SUS_HTTP_REQUEST) { 0 };
	}
	SUS_PRINTDL("The request was created successfully!");
	return req;
}

// Add a header to a query
BOOL SUSAPI susHttpSetHeader(_Inout_ HINTERNET hRequest, _In_ LPCWSTR key, _In_ LPCWSTR value, _In_ BOOL replace)
{
	SUS_PRINTDL("Setting the http request header");
	SUS_ASSERT(hRequest && key && value);
	LPWSTR header = sus_fcalloc(sus_formattingW(NULL, L"%s: %s", key, value) + 1, sizeof(WCHAR));
	if (!header) return FALSE;
	sus_formattingW(header, L"%s: %s", key, value);
	if (!WinHttpAddRequestHeaders(hRequest, header, (DWORD)-1, replace ? WINHTTP_ADDREQ_FLAG_REPLACE : WINHTTP_ADDREQ_FLAG_ADD)) {
		sus_free(header);
		SUS_PRINTDE("Couldn't set the header");
		SUS_PRINTDC(GetLastError());
		return FALSE;
	}
	sus_free(header);
	SUS_PRINTDL("Headers have been successfully installed");
	return TRUE;
}

// Send a request
BOOL SUSAPI susHttpRequestExecute(_In_ HINTERNET hRequest, _In_opt_ LPCWSTR lpszHeaders, _In_opt_ SUS_DATAVIEW body)
{
	SUS_PRINTDL("Sending an http request");
	SUS_ASSERT(hRequest);
	if (body.data && body.size) {
		WCHAR len[20] = { 0 };
		sus_itow(len, body.size);
		susHttpSetHeader(hRequest, L"Content-Length", len, FALSE);
	}
	if (!WinHttpSendRequest(hRequest, lpszHeaders, (DWORD)-1, NULL, 0, (DWORD)body.size, 0)) {
		SUS_PRINTDE("Couldn't send request");
		SUS_PRINTDC(GetLastError());
		return FALSE;
	}
	if (body.data) {
		for (SIZE_T sent = 0, written = 1; sent < body.size; sent += written) {
			if (!WinHttpWriteData(hRequest, body.data + sent, (DWORD)min(body.size - sent, 1024), (LPDWORD)&written)) return FALSE;
			if (!written) break;
		}
	}
	SUS_PRINTDL("The request has been sent successfully!");
	return TRUE;
}

// Get a response from the server
SUS_HTTP_RESPONSE SUSAPI susHttpReceiveResponse(_In_ HINTERNET hRequest)
{
	SUS_PRINTDL("Getting a response from the server");
	SUS_ASSERT(hRequest);
	SUS_HTTP_RESPONSE res = (SUS_HTTP_RESPONSE){ 0 };
	if (!WinHttpReceiveResponse(hRequest, NULL)) {
		SUS_PRINTDE("Failed to request a response");
		SUS_PRINTDC(GetLastError());
		return (SUS_HTTP_RESPONSE) { 0 };
	}
	susHttpQueryBody(hRequest, &res.body.content);
	susHttpQueryHeaders(hRequest, &res.headers);
	susHttpQueryStatusCode(hRequest, (LPDWORD)&res.dwStatus);
	susHttpQueryStatusMessage(hRequest, res.message, sizeof(res.message));
	susHttpQueryTime(hRequest, &res.stTime);
	res.body.type = susHttpQueryContentType(hRequest);
	res.body.encoding = susHttpQueryContentEncoding(hRequest);
	return res;
}

// Request a request
SUS_HTTP_RESPONSE SUSAPI susHttpRequest(_In_ LPCWSTR method, _In_ LPCWSTR url, _In_opt_ LPCWSTR lpszHeaders, _In_opt_ SUS_DATAVIEW body)
{
	SUS_WPRINTDL("Sending a '%s' request to the url: %s", method, url);
	SUS_ASSERT(method && url);
	SUS_HTTP_REQUEST req = susHttpRequestSetup(method, url);
	if (!req.hConnect || !req.hRequest) return (SUS_HTTP_RESPONSE) { 0 };
	if (!susHttpRequestExecute(req.hRequest, lpszHeaders, body)) {
		susHttpRequestClose(&req);
		SUS_PRINTDE("Couldn't make a request");
		return (SUS_HTTP_RESPONSE) { 0 };
	}
	SUS_HTTP_RESPONSE res = susHttpReceiveResponse(req.hRequest);
	if (!res.dwStatus) {
		susHttpRequestClose(&req);
		SUS_PRINTDE("Couldn't make a request");
		return (SUS_HTTP_RESPONSE) { 0 };
	}
	susHttpRequestClose(&req);
	return res;
}

// ------------------------------------------------------------

// Close the http request
VOID SUSAPI susHttpRequestClose(_Inout_ SUS_LPHTTP_REQUEST req)
{
	WinHttpCloseHandle(req->hRequest);
	req->hRequest = NULL;
	WinHttpCloseHandle(req->hConnect);
	req->hConnect = NULL;
}
// Clear the response data
VOID SUSAPI susHttpResponseCleanup(_Inout_ SUS_LPHTTP_RESPONSE res)
{
	if (res->headers) {
		susMapForeach(res->headers, entry) {
			susBufferDestroy(*(SUS_BUFFER*)susMapValue(res->headers, entry));
		}
		susMapDestroy(res->headers);
	}
	res->headers = NULL;
	if (res->body.content.data) susDataDestroy(res->body.content);
	res->body.content.data = NULL;
}