// httprequest.h
//
#include "coreframe.h"
#include "include/susfwk/core.h"
#include "include/susfwk/memory.h"
#include "include/susfwk/hashtable.h"
#include "include/susfwk/httprequest.h"

// 
SUS_STATIC HINTERNET hSession = NULL;

// =================================================================================================

// configuring the http protocol
BOOL SUSAPI susHttpSetup()
{
	SUS_PRINTDL("");
	SUS_ASSERT(!hSession);
	hSession = WinHttpOpen(
		L"SUS-HTTP-CLIENT/1.0",
		WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
		WINHTTP_NO_PROXY_NAME,
		WINHTTP_NO_PROXY_BYPASS,
		0
	);
	return (BOOL)(ULONG_PTR)hSession;
}
// Clearing http protocol data
VOID SUSAPI susHttpCleanup()
{
	SUS_PRINTDL("");
	SUS_ASSERT(hSession);
	WinHttpCloseHandle(hSession);
	hSession = NULL;
}

// =================================================================================================

// URL structure
typedef struct sus_http_url {
	INTERNET_PORT port;
	WCHAR host[64];
	WCHAR path[MAX_PATH];
} SUS_HTTP_URL, * SUS_LPHTTP_URL;
// Parse the URL
static BOOL SUSAPI susHttpParseUrl(_In_ LPCWSTR textUrl, _Out_ SUS_LPHTTP_URL url)
{
	SUS_ASSERT(textUrl && url);
	URL_COMPONENTS uc = { 0 };
	uc.dwStructSize = sizeof(uc);
	uc.dwHostNameLength = 1;
	uc.dwUrlPathLength = 1;
	uc.dwExtraInfoLength = 1;
	if (!WinHttpCrackUrl(textUrl, 0, 0, &uc)) {
		*url = (SUS_HTTP_URL){ 0 };
		return FALSE;
	}
	if (uc.lpszHostName) {
		sus_memcpy((LPBYTE)url->host, (LPBYTE)uc.lpszHostName, uc.dwHostNameLength * sizeof(WCHAR));
		url->host[uc.dwHostNameLength] = L'\0';
	}
	if (uc.lpszUrlPath) sus_memcpy((LPBYTE)url->path, (LPBYTE)uc.lpszUrlPath, uc.dwUrlPathLength * sizeof(WCHAR));
	if (uc.lpszExtraInfo && uc.dwExtraInfoLength) {
		sus_memcpy((LPBYTE)url->path + uc.dwUrlPathLength, (LPBYTE)uc.lpszExtraInfo, min(uc.dwExtraInfoLength * sizeof(WCHAR), sizeof(url->path)));
	}
	url->path[min(uc.dwExtraInfoLength + uc.dwUrlPathLength, sizeof(url->path) / sizeof(WCHAR) - 1)] = L'\0';
	url->port = uc.nPort;
	return TRUE;
}
// Create a request
BOOL SUSAPI susHttpRequestSetup(_In_ LPCWSTR method, _In_ LPCWSTR lpUrl, _Out_ SUS_LPHTTP_REQUEST req)
{
	SUS_PRINTDL("Creating a request");
	SUS_ASSERT(method && lpUrl);
	SUS_HTTP_URL url = { 0 };
	if (!susHttpParseUrl(lpUrl, &url)) {
		*req = (SUS_HTTP_REQUEST){ 0 };
		SUS_PRINTDE("Failed to create a request");
		return FALSE;
	}
	req->hConnect = WinHttpConnect(hSession, url.host, url.port, 0);
	if (!req->hConnect) {
		*req = (SUS_HTTP_REQUEST){ 0 };
		SUS_PRINTDE("Failed to create a request");
		return FALSE;
	}
	req->hRequest = WinHttpOpenRequest(
		req->hConnect, method, url.path, NULL,
		WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES,
		(url.port == INTERNET_DEFAULT_HTTPS_PORT) ? WINHTTP_FLAG_SECURE : 0
	);
	if (!req->hRequest) {
		WinHttpCloseHandle(req->hConnect);
		*req = (SUS_HTTP_REQUEST){ 0 };
		SUS_PRINTDE("Failed to create a request");
		return FALSE;
	}
	return TRUE;
}

// Add a header to a query
BOOL SUSAPI susHttpAddHeader(_Inout_ HINTERNET hRequest, LPCWSTR key, LPCWSTR value)
{
	SUS_PRINTDL("");
	SUS_ASSERT(hRequest && key && value);
	LPWSTR header = sus_fcalloc(sus_formattingW(NULL, L"%s: %s", key, value) + 1, sizeof(WCHAR));
	if (!header) return FALSE;
	sus_formattingW(header, L"%s: %s", key, value);
	if (!WinHttpAddRequestHeaders(hRequest, header, (DWORD)-1, WINHTTP_ADDREQ_FLAG_ADD)) {
		SUS_PRINTDE("");
		SUS_PRINTDC(GetLastError());
		return FALSE;
	}
	sus_free(header);
	return TRUE;
}
// Send a request
BOOL SUSAPI susHttpSendRequest(_In_ HINTERNET hRequest, _In_opt_ SUS_DATAVIEW body)
{
	SUS_PRINTDL("Sending an http request");
	SUS_ASSERT(hRequest);
	if (body.data && body.size) {
		WCHAR len[16] = { 0 };
		sus_itow(len, body.size);
		susHttpAddHeader(hRequest, L"Content-Length", len);
	}
	if (!WinHttpSendRequest(hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0, NULL, 0, (DWORD)body.size, 0)) {
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
// 
static BOOL SUSAPI susHttpParseHeaderLine(_Inout_ SUS_LPHASHMAP map, _In_ LPWSTR line) {
	LPWSTR colon = sus_strchrW(line, L':');
	if (!colon) return FALSE;
	*colon = L'\0';
	susMapAdd(map, sus_trimW(line), sus_trimW(colon + 1));
	*colon = L':';
	return TRUE;
}
// 
static VOID SUSAPI susHttpParseHeaders(_In_ LPWSTR pwszHeaders, _Inout_ SUS_LPHASHMAP headers) {
	SUS_ASSERT(pwszHeaders && headers);
	for (LPWSTR line = pwszHeaders; *line;) {
		LPWSTR end = sus_strstrW(line, L"\r\n");
		if (!end || end == line) break;
		*end = L'\0';
		susHttpParseHeaderLine(headers, line);
		*end = L'\r';
		line = end + 2;
	}
}

// 
static VOID SUSAPI susHttpQueryStatusCode(_In_ HINTERNET hRequest, _Out_ LPDWORD pdwStatus) {
	SUS_ASSERT(hRequest && pdwStatus);
	DWORD dwStatusSize = sizeof(*pdwStatus);
	WinHttpQueryHeaders(hRequest,
		WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER,
		WINHTTP_HEADER_NAME_BY_INDEX, pdwStatus,
		&dwStatusSize, WINHTTP_NO_HEADER_INDEX
	);
}
// 
static VOID SUSAPI susHttpQueryStatusMessage(_In_ HINTERNET hRequest, _Out_writes_bytes_(size + sizeof(WCHAR)) LPWSTR msg, _In_ DWORD size) {
	SUS_ASSERT(hRequest && msg);
	WinHttpQueryHeaders(hRequest,
		WINHTTP_QUERY_STATUS_TEXT,
		WINHTTP_HEADER_NAME_BY_INDEX, msg,
		&size, WINHTTP_NO_HEADER_INDEX
	);
	*((LPWSTR)(msg + size)) = L'\0';
}
// 
static VOID SUSAPI susHttpQueryTime(_In_ HINTERNET hRequest, _Out_ LPSYSTEMTIME time) {
	SUS_ASSERT(hRequest && time);
	DWORD dwTimeSize = sizeof(*time);
	WinHttpQueryHeaders(hRequest,
		WINHTTP_QUERY_DATE | WINHTTP_QUERY_FLAG_SYSTEMTIME,
		WINHTTP_HEADER_NAME_BY_INDEX, time,
		&dwTimeSize, WINHTTP_NO_HEADER_INDEX
	);
}
// 
static BOOL SUSAPI susHttpQueryHeaders(_In_ HINTERNET hRequest, _Inout_ SUS_LPHASHMAP headers) {
	SUS_ASSERT(hRequest && headers);
	DWORD dwHeaderSize = 0;
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
		susHttpParseHeaders(pwszHeaders, headers);
		sus_free(pwszHeaders);
		return TRUE;
	}
	*headers = NULL;
	SUS_PRINTDE("Couldn't get the headers");
	SUS_PRINTDC(GetLastError());
	return FALSE;
}
// 
static BOOL SUSAPI susHttpQueryBody(_In_ HINTERNET hRequest, _Out_ SUS_LPBUFFER body) {
	*body = susNewBuffer(256);
	DWORD dwAvailable = 0;
	while (WinHttpQueryDataAvailable(hRequest, &dwAvailable) && dwAvailable) {
		DWORD read = 0;
		if (!WinHttpReadData(hRequest, susBufferAppend(body, NULL, dwAvailable), dwAvailable, &read) || !read) {
			susBufferDestroy(*body);
			SUS_PRINTDE("Couldn't read response body data");
			return FALSE;
		}
	}
	return TRUE;
}

// Get a response from the server
BOOL SUSAPI susHttpParseResponse(_In_ HINTERNET hRequest, _Out_ SUS_LPHTTP_RESPONSE response)
{
	SUS_PRINTDL("Getting a response from the server");
	SUS_ASSERT(hRequest);
	*response = (SUS_HTTP_RESPONSE){ 0 };
	if (!WinHttpReceiveResponse(hRequest, NULL)) {
		SUS_PRINTDE("Failed to request a response");
		SUS_PRINTDC(GetLastError());
		return FALSE;
	}
	if (!susHttpQueryBody(hRequest, &response->body)) return FALSE;
	response->headers = susNewMap(WCHAR[32], WCHAR[64]);
	susHttpQueryHeaders(hRequest, &response->headers);
	susHttpQueryStatusCode(hRequest, (LPDWORD)&response->status);
	susHttpQueryStatusMessage(hRequest, response->message, sizeof(response->message) - sizeof(WCHAR));
	susHttpQueryTime(hRequest, &response->stReceived);
	return TRUE;
}
// Make a request
SUS_HTTP_RESPONSE SUSAPI susHttpPerformRequest(SUS_HTTP_REQUEST req, _In_opt_ SUS_DATAVIEW body)
{
	susHttpSendRequest(req.hRequest, body);
	SUS_HTTP_RESPONSE res;
	susHttpParseResponse(req.hRequest, &res);
	WinHttpCloseHandle(req.hRequest);
	WinHttpCloseHandle(req.hConnect);
	return res;
}
