// httprequest.h
//
#include "coreframe.h"
#include "include/susfwk/core.h"
#include "include/susfwk/memory.h"
#include "include/susfwk/hashtable.h"
#include "include/susfwk/httprequest.h"

// 
SUS_STATIC HINTERNET hSession = NULL;

// 
static BOOL SUSAPI susHttpParseUrl(LPCWSTR textUrl, _Out_ SUS_LPHTTP_URL url)
{
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
		sus_memcpy((LPBYTE)url->path + uc.dwUrlPathLength, (LPBYTE)uc.lpszExtraInfo, uc.dwExtraInfoLength * sizeof(WCHAR));
	}
	url->path[uc.dwUrlPathLength + uc.dwExtraInfoLength] = L'\0';
	url->port = uc.nPort;
	return TRUE;
}

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

// 
SUS_HTTP_REQUEST SUSAPI susHttpRequestSetup(_In_ LPCWSTR method, _In_ LPCWSTR lpUrl)
{
	SUS_PRINTDL("");
	SUS_ASSERT(method && lpUrl);
	SUS_HTTP_URL url = { 0 };
	if (!susHttpParseUrl(lpUrl, &url)) {
		SUS_PRINTDE("");
		return (SUS_HTTP_REQUEST) { 0 };
	}
	SUS_HTTP_REQUEST req;
	req.hConnect = WinHttpConnect(hSession, url.host, url.port, 0);
	if (!req.hConnect) {
		SUS_PRINTDE("");
		return (SUS_HTTP_REQUEST) { 0 };
	}
	req.hRequest = WinHttpOpenRequest(
		req.hConnect, method, url.path, NULL,
		WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES,
		(url.port == INTERNET_DEFAULT_HTTPS_PORT) ? WINHTTP_FLAG_SECURE : 0
	);
	if (!req.hRequest) {
		WinHttpCloseHandle(req.hConnect);
		SUS_PRINTDE("");
		return (SUS_HTTP_REQUEST) { 0 };
	}
	return req;
}

// 
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
// 
BOOL SUSAPI susHttpSendRequest(_In_ HINTERNET hRequest, _In_opt_ SUS_DATAVIEW body)
{
	SUS_PRINTDL("");
	SUS_ASSERT(hRequest);
	if (body.data && body.size) {
		WCHAR len[16] = { 0 };
		sus_itow(len, body.size);
		susHttpAddHeader(hRequest, L"Content-Length", len);
	}
	if (!WinHttpSendRequest(hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0, NULL, 0, (DWORD)body.size, 0)) {
		SUS_PRINTDE("");
		SUS_PRINTDC(GetLastError());
		return FALSE;
	}
	if (body.data) {
		for (SIZE_T sent = 0, written = 1; sent < body.size; sent += written) {
			if (!WinHttpWriteData(hRequest, body.data + sent, (DWORD)min(body.size - sent, 1024), (LPDWORD)&written)) return FALSE;
			if (!written) break;
		}
	}
	if (!WinHttpReceiveResponse(hRequest, NULL)) {
		SUS_PRINTDE("");
		SUS_PRINTDC(GetLastError());
		return FALSE;
	}
	return TRUE;
}
// 
SUS_STATIC BOOL SUSAPI susHttpParseHeaderLine(_Inout_ SUS_LPHASHMAP map, _In_ LPWSTR line) {
	LPWSTR colon = sus_strchrW(line, L':');
	if (!colon) return FALSE;
	*colon = L'\0';
	susMapAdd(map, sus_trimW(line), sus_trimW(colon + 1));
	*colon = L':';
	return TRUE;
}
// 
SUS_STATIC SUS_HASHMAP SUSAPI susHttpParseHeaders(_In_ LPCWSTR pwszHeaders) {
	SUS_ASSERT(pwszHeaders);
	SUS_HASHMAP map = susNewMap(LPWSTR, LPWSTR);
	if (!map) {
		return NULL;
	}
	LPWSTR mutableHeaders = sus_calloc(sus_strlenW(pwszHeaders) + 1, sizeof(WCHAR));
	if (!mutableHeaders) {
		susMapDestroy(map);
		return NULL;
	}
	sus_strcpyW(mutableHeaders, pwszHeaders);
	for (LPWSTR line = mutableHeaders; *line;) {
		LPWSTR end = sus_strstrW(line, L"\r\n");
		if (!end || end == line) break;
		*end = L'\0';
		susHttpParseHeaderLine(&map, line);
		*end = L'\r';
		line = end + 2;
	}
	sus_free(mutableHeaders);
	return map;
}
// 
SUS_HTTP_RESPONSE SUSAPI susHttpGetResponse(_In_ HINTERNET hRequest)
{
	SUS_PRINTDL("");
	SUS_ASSERT(hRequest);
	SUS_HTTP_RESPONSE res = { 0 };
	DWORD dwStatusSize = sizeof(res.status);
	WinHttpQueryHeaders(hRequest,
		WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER,
		WINHTTP_HEADER_NAME_BY_INDEX, &res.status,
		&dwStatusSize, WINHTTP_NO_HEADER_INDEX
	);
	DWORD dwMsgSize = sizeof(res.message) - sizeof(WCHAR);
	WinHttpQueryHeaders(hRequest,
		WINHTTP_QUERY_STATUS_TEXT,
		WINHTTP_HEADER_NAME_BY_INDEX, res.message,
		&dwMsgSize, WINHTTP_NO_HEADER_INDEX
	);
	DWORD dwTimeSize = sizeof(res.stReceived);
	WinHttpQueryHeaders(hRequest,
		WINHTTP_QUERY_DATE | WINHTTP_QUERY_FLAG_SYSTEMTIME,
		WINHTTP_HEADER_NAME_BY_INDEX, &res.stReceived,
		&dwTimeSize, WINHTTP_NO_HEADER_INDEX
	);

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
		res.headers = susHttpParseHeaders(pwszHeaders);
		sus_free(pwszHeaders);
	}
	SUS_BUFFER bodyBuffer = susNewBuffer(1024);
	DWORD dwSize = 0;
	do {
		DWORD dwDownloaded = 0;
		if (!WinHttpQueryDataAvailable(hRequest, &dwSize)) {
			SUS_PRINTDE("");
			break;
		}
		if (!dwSize) break;
		if (!WinHttpReadData(hRequest, susBufferAppend(&bodyBuffer, NULL, dwSize), dwSize, &dwDownloaded)) {
			SUS_PRINTDE("");
			break;
		}
	} while (dwSize);
	res.body = susNewData(bodyBuffer->size);
	if (res.body.data) sus_memcpy(res.body.data, (LPBYTE)bodyBuffer->data, bodyBuffer->size);
	susBufferDestroy(bodyBuffer);
	return res;
}
//
SUS_HTTP_RESPONSE SUSAPI susHttpPerformRequest(SUS_HTTP_REQUEST req, _In_opt_ SUS_DATAVIEW body)
{
	susHttpSendRequest(req.hRequest, body);
	SUS_HTTP_RESPONSE res = susHttpGetResponse(req.hRequest);
	WinHttpCloseHandle(req.hRequest);
	WinHttpCloseHandle(req.hConnect);
	return res;
}
