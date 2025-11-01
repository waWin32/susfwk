// jnet.c
//
#include "include/susfwk/core.h"
#include "include/susfwk/json.h"
#include "include/susfwk/network.h"
#include "include/susfwk/jnet.h"

// ================================================================================================

// Create a new JNET object
SUS_JNET SUSAPI susNewJnet(_In_opt_ SUS_OBJECT userData)
{
	SUS_PRINTDL("Initializing the JNET protocol on a socket");
	SUS_JNET jnet = sus_calloc(1, sizeof(SUS_JNET_STRUCT));
	if (!jnet) return NULL;
	jnet->userData = userData;
	return jnet;
}
// Delete a JNET object
VOID SUSAPI susJnetDestroy(_In_ SUS_JNET jnet)
{
	SUS_PRINTDL("Cleaning the JNET protocol on a socket");
	SUS_ASSERT(jnet);
	sus_free(jnet);
}

// ================================================================================================

// Incoming data handler
static BOOL SUSAPI susJnetDataHandler(_In_ SUS_LPSOCKET sock, _In_ LPCSTR data)
{
	SUS_PRINTDL("Processing of received data");
	SUS_ASSERT(sock && data);
	SUS_JNET jNetSock = (SUS_JNET)susSocketGetUserData(sock);
	SUS_ASSERT(jNetSock);
	SUS_JSON json = susJsonParse(data);
	if (json.type != SUS_JSON_TYPE_OBJECT) goto incorrect_data;
	SUS_LPJSON headers = susJsonObjectGet(json, "headers");
	SUS_LPJSON body = susJsonObjectGet(json, "body");
	SUS_LPJSON id = susJsonObjectGet(json, "id");
	if (id) {
		SUS_LPJSON status = susJsonObjectGet(json, "status");
		if (status) {
			if (status->type != SUS_JSON_TYPE_NUMBER) goto incorrect_data;
			SUS_PRINTDL("Received a response");
			if (jNetSock->resHandler) jNetSock->resHandler(sock, id, (INT)status->value.number, headers ? headers : &susJsonNull(), body ? body : &susJsonNull(), jNetSock->userData);
		}
		else {
			SUS_PRINTDL("Request received");
			if (jNetSock->reqHandler) {
				SUS_LPJSON path = susJsonObjectGet(json, "path");
				if (!path || path->type != SUS_JSON_TYPE_STRING) goto incorrect_data;
				SUS_LPJSON method = susJsonObjectGet(json, "method");
				if (!method || method->type != SUS_JSON_TYPE_NUMBER) goto incorrect_data;
				SUS_JSON res = jNetSock->reqHandler(sock, id, (UINT)method->value.number, path->value.str, headers ? headers : &susJsonNull(), body ? body : &susJsonNull(), jNetSock->userData);
				susJnetSend(sock, res);
				susJsonDestroy(&res);
			}
		}
	}
	else {
		SUS_PRINTDL("Message received");
		SUS_LPJSON path = susJsonObjectGet(json, "path");
		if (!path || path->type != SUS_JSON_TYPE_STRING) goto incorrect_data;
		if (jNetSock->msgHandler) jNetSock->msgHandler(sock, path->value.str, headers ? headers : &susJsonNull(), body ? body : &susJsonNull(), jNetSock->userData);
	}
	susJsonDestroy(&json);
	return TRUE;
incorrect_data:
	susJsonDestroy(&json);
	SUS_PRINTDW("Incorrect JNET format received");
	return FALSE;
}
// JNET Socket Message Handler
BOOL SUSAPI susJnetSocketHandler(_In_ SUS_LPSOCKET sock, _In_ SUS_SOCKET_MESSAGE uMsg, _In_ WPARAM wParam, _In_ LPARAM lParam)
{
	UNREFERENCED_PARAMETER(wParam);
	switch (uMsg)
	{
	case SUS_SM_DATA:
		return susJnetDataHandler(sock, (LPCSTR)lParam);
	case SUS_SM_CREATE:
		susSocketSetUserData(sock, susNewJnet(susSocketGetUserData(sock)));
		return susSocketGetUserData(sock) ? TRUE : FALSE;
	case SUS_SM_END:
		susJnetDestroy(susSocketGetUserData(sock));
		return TRUE;
	default: return TRUE;
	}
}

// ================================================================================================

// Send json to the socket
BOOL SUSAPI susJnetSend(_Inout_ SUS_LPSOCKET sock, _In_ SUS_JSON json)
{
	SUS_ASSERT(sock);
	LPSTR jsonText = susJsonStringify(json);
	if (!jsonText) return FALSE;
	susSocketWriteText(sock, jsonText);
	sus_strfree(jsonText);
	return TRUE;
}

// Create a JNET request
SUS_JSON SUSAPI susJnetRequestSetup(_In_ SUS_JSON id, _In_ SUS_JNET_METHOD method, _In_opt_ LPCSTR path, _In_opt_ SUS_JSON headers, _In_opt_ SUS_JSON body)
{
	SUS_PRINTDL("Creating a request");
	SUS_JSON req = susJsonObject();
	if (!susJsonIsValid(req)) return susJsonNull();
	susJsonObjectSet(&req, "id", id);
	susJsonObjectSet(&req, "path", susJsonStringView(path));
	susJsonObjectSet(&req, "method", susJsonNumber(method));
	susJsonObjectSet(&req, "headers", headers);
	susJsonObjectSet(&req, "body", body);
	return req;
}
// Create a JNET message
SUS_JSON SUSAPI susJnetNotificationSetup(_In_opt_ LPCSTR path, _In_opt_ SUS_JSON headers, _In_opt_ SUS_JSON body)
{
	SUS_PRINTDL("Creating a message");
	SUS_JSON msg = susJsonObject();
	if (!susJsonIsValid(msg)) return susJsonNull();
	susJsonObjectSet(&msg, "path", susJsonStringView(path));
	susJsonObjectSet(&msg, "headers", headers);
	susJsonObjectSet(&msg, "body", body);
	return msg;
}
// Create a JNET response
SUS_JSON SUSAPI susJnetResponseSetup(_In_ SUS_JSON id, _In_ INT status, _In_opt_ SUS_JSON headers, _In_opt_ SUS_JSON body)
{
	SUS_PRINTDL("Creating a response");
	SUS_JSON res = susJsonObject();
	if (!susJsonIsValid(res)) return susJsonNull();
	susJsonObjectSet(&res, "id", id);
	susJsonObjectSet(&res, "status", susJsonNumber((FLOAT)status));
	susJsonObjectSet(&res, "headers", headers);
	susJsonObjectSet(&res, "body", body);
	return res;
}

// Install handlers for the jnet socket
VOID SUSAPI susJnetSetHandler(_Inout_ SUS_LPSOCKET sock, _In_opt_ SUS_JNET_RESPONSE_HANDLER resHandler, _In_opt_ SUS_JNET_REQUEST_HANDLER reqHandler, _In_opt_ SUS_JNET_NOTIFICATION_HANDLER msgHandler)
{
	SUS_ASSERT(sock && sock->userData);
	SUS_JNET jnet = susSocketGetUserData(sock);
	jnet->reqHandler = reqHandler;
	jnet->resHandler = resHandler;
	jnet->msgHandler = msgHandler;
}

// ================================================================================================
