// json.c
//
#include "coreframe.h"
#include "include/susfwk/core.h"
#include "include/susfwk/memory.h"
#include "include/susfwk/vector.h"
#include "include/susfwk/hashtable.h"
#include "include/susfwk/math.h"
#include "include/susfwk/json.h"

// =======================================================================================

// -----------------------------------------------

// Delete a json object
VOID SUSAPI susJsonDestroy(_Inout_ SUS_LPJSON json) {
	SUS_PRINTDL("Deleting a json object");
	if (!susJsonIsValid(*json)) return;
	switch (json->type)
	{
	case SUS_JSON_TYPE_STRING: {
		sus_strfree(json->value.str);
	} break;
	case SUS_JSON_TYPE_ARRAY: {
		susVecForeach(i, json->value.array) {
			susJsonDestroy((SUS_LPJSON)susVectorGet(json->value.array, i));
		}
		susVectorDestroy(json->value.array);
	} break;
	case SUS_JSON_TYPE_OBJECT: {
		susMapForeach(json->value.object, i) {
			sus_strfree(*(LPSTR*)susMapIterKey(i));
			susJsonDestroy((SUS_LPJSON)susMapIterValue(i));
		}
		susMapDestroy(json->value.object);
	} break;
	}
	*json = susJsonNull();
}

// Deep copying of a json object
SUS_JSON SUSAPI susJsonCopy(_In_ SUS_JSON json)
{
	SUS_PRINTDL("Copying a json object");
	SUS_JSON jsonCopy = susJsonNull();
	switch (json.type)
	{
	case SUS_JSON_TYPE_STRING_VIEW:
	case SUS_JSON_TYPE_STRING: {
		jsonCopy = susJsonString(json.value.str);
	} break;
	case SUS_JSON_TYPE_ARRAY: {
		jsonCopy = susJsonArray();
		susVecForeach(i, json.value.array) {
			SUS_LPJSON obj = susVectorGet(json.value.array, i);
			if (!obj) continue;
			susJsonArrayPush(&jsonCopy, susJsonCopy(*obj));
		}
	} break;
	case SUS_JSON_TYPE_OBJECT: {
		jsonCopy = susJsonObject();
		susMapForeach(json.value.object, i) {
			susJsonObjectSet(&jsonCopy, *(LPSTR*)susMapIterKey(i), susJsonCopy(*(SUS_LPJSON)susMapIterValue(i)));
		}
	} break;
	default: {
		jsonCopy = json;
	} break;
	}
	return jsonCopy;
}

// -----------------------------------------------

// Convert json string to string
static VOID SUSAPI susJsonStringStringify(_In_ LPCSTR str, _Inout_ SUS_LPBUFFER pBuffer) {
	SUS_ASSERT(pBuffer && *pBuffer);
	susBufferAppend(pBuffer, (LPBYTE)"\"", 1);
	LPSTR buff = sus_malloc(((SIZE_T)lstrlenA(str) + 16) * sizeof(CHAR));
	if (!buff) return;
	sus_escapeA(buff, str);
	susBufferAppend(pBuffer, (LPBYTE)buff, lstrlenA(buff) * sizeof(CHAR));
	susBufferAppend(pBuffer, (LPBYTE)"\"", 1);
	sus_free(buff);
}
// Convert Json to a string recursively
static VOID SUSAPI susJsonStringifyRecursively(_In_ SUS_LPJSON json, _Inout_ SUS_LPBUFFER pBuffer)
{
	SUS_ASSERT(pBuffer && *pBuffer);
	switch (json->type)
	{
	case SUS_JSON_TYPE_STRING_VIEW:
	case SUS_JSON_TYPE_STRING: {
		susJsonStringStringify(json->value.str, pBuffer);
	} break;
	case SUS_JSON_TYPE_NUMBER: {
		CHAR buff[32];
		sus_ftoa(buff, json->value.number, 6);
		susBufferAppend(pBuffer, (LPBYTE)buff, lstrlenA(buff) * sizeof(CHAR));
	} break;
	case SUS_JSON_TYPE_BOOLEAN: {
		if (json->value.boolean) susBufferAppend(pBuffer, (LPBYTE)"true", 4);
		else susBufferAppend(pBuffer, (LPBYTE)"false", 5);
	} break;
	case SUS_JSON_TYPE_ARRAY: {
		susBufferAppend(pBuffer, (LPBYTE)"[", sizeof(CHAR));
		susVecForeach(i, json->value.array) {
			SUS_LPJSON obj = susVectorGet(json->value.array, i);
			if (!obj) continue;
			susJsonStringifyRecursively(obj, pBuffer);
			if (i < json->value.array->length - 1) susBufferAppend(pBuffer, (LPBYTE)", ", 2);
		}
		susBufferAppend(pBuffer, (LPBYTE)"]", sizeof(CHAR));
	} break;
	case SUS_JSON_TYPE_OBJECT: {
		susBufferAppend(pBuffer, (LPBYTE)"{", sizeof(CHAR));
		susMapForeach(json->value.object, i) {
			susJsonStringStringify(*(LPSTR*)susMapIterKey(i), pBuffer);
			susBufferAppend(pBuffer, (LPBYTE)": ", 2);
			SUS_LPJSON obj = susMapIterValue(i);
			susJsonStringifyRecursively(obj, pBuffer);
			if (i.count < json->value.object->count - 1) susBufferAppend(pBuffer, (LPBYTE)", ", 2);
		}
		susBufferAppend(pBuffer, (LPBYTE)"}", sizeof(CHAR));
	} break;
	default: {
		susBufferAppend(pBuffer, (LPBYTE)"null", sizeof("null") - sizeof(CHAR));
	} break;
	}
}
// Convert json to string
LPSTR SUSAPI susJsonStringify(_In_ SUS_JSON json)
{
	SUS_PRINTDL("Converting a json object to a string");
	SUS_BUFFER buffer = susNewBuffer(256);
	if (!buffer) return NULL;
	susJsonStringifyRecursively(&json, &buffer);
	susBufferAppend(&buffer, (LPBYTE)"\0", sizeof(CHAR));
	LPSTR stringify = sus_strdup((LPSTR)buffer->data);
	susBufferDestroy(buffer);
	return stringify;
}

// Convert json string to string
static LPSTR SUSAPI susJsonStringParse(_Inout_ LPSTR* text) {
	SUS_ASSERT(text && *text && **text == '"');
	LPSTR end = sus_strchrA(++(*text), '"');
	while (end && *(end - 1) == '\\' && *(end - 2) != '\\') end = sus_strchrA(end + 1, '"');
	if (!end) return NULL;
	*end = '\0';
	LPSTR buff = sus_malloc((SIZE_T)sus_unescapeA(NULL, *text) + 1);
	if (!buff) {
		*end = '"';
		return NULL;
	}
	sus_unescapeA(buff, *text);
	*end = '"';
	*text = end + 1;
	return buff;
}
// Convert string to json
static SUS_JSON SUSAPI susParseJsonValue(_In_ LPSTR* text)
{
	SUS_ASSERT(text);
	sus_trimlA(text);
	SUS_JSON json = susJsonNull();
	if (!**text) return json;
	switch (*(*text))
	{
	case '"': {
		LPSTR str = susJsonStringParse(text);
		json = susJsonString(str);
		sus_strfree(str);
	} break;
	case 't':
	case 'f': {
		if (lstrcmpA(*text, "true") == 0) {
			*text += 4;
			json = susJsonBoolean(TRUE);
		}
		else {
			*text += 5;
			json = susJsonBoolean(FALSE);
		}
	}  break;
	case '{': {
		json = susJsonObject();
		(*text)++;
		while (**text && **text != '}') {
			sus_trimlA(text);
			if (**text != '"') {
				SUS_PRINTDE("Does not match the json format");
				susJsonDestroy(&json);
				json = susJsonNull();
				break;
			}
			LPSTR key = susJsonStringParse(text);
			sus_trimlA(text);
			if (**text != ':') {
				SUS_PRINTDE("Does not match the json format");
				susJsonDestroy(&json);
				json = susJsonNull();
				sus_strfree(key);
				break;
			}
			(*text)++;
			susJsonObjectSet(&json, key, susParseJsonValue(text));
			sus_strfree(key);
			sus_trimlA(text);
			if (**text == ',') {
				(*text)++;
			}
		}
		(*text)++;
	} break;
	case '[': {
		json = susJsonArray();
		(*text)++;
		while (**text && **text != ']') {
			susJsonArrayPush(&json, susParseJsonValue(text));
			sus_trimlA(text);
			if (**text == ',') {
				(*text)++;
			}
		}
		(*text)++;
	} break;
	case 'n': {
		if (!sus_memcmp((LPBYTE)*text, (LPBYTE)"null", 4)) {
			SUS_PRINTDE("Does not match the json format");
			break;
		}
		(*text) += 4;
	} break;
	default: {
		if (sus_isdigitA(**text)) {
			json = susJsonNumber(sus_atof(*text, text));
		}
	};
	}
	return json;
}
// Convert string to json
SUS_JSON SUSAPI susJsonParse(_In_ LPCSTR text)
{
	SUS_PRINTDL("parsing a string in json format");
	SUS_ASSERT(text);
	LPSTR str = sus_strdup(text);
	if (!str) return susJsonNull();
	LPSTR ctx = str;
	SUS_JSON json = susParseJsonValue(&ctx);
	sus_strfree(str);
	return json;
}

// -----------------------------------------------

// Comparison of 2 json objects
BOOL SUSAPI susJsonEquals(_In_ SUS_JSON a, _In_ SUS_JSON b)
{
	SUS_PRINTDL("Comparing two json objects");
	if (a.type != b.type && !((a.type == SUS_JSON_TYPE_STRING_VIEW && b.type == SUS_JSON_TYPE_STRING) || (a.type == SUS_JSON_TYPE_STRING && b.type == SUS_JSON_TYPE_STRING_VIEW))) return FALSE;
	switch (a.type)
	{
	case SUS_JSON_TYPE_NUMBER:
		return sus_fabs(a.value.number - b.value.number) < SUS_EPSILON;
	case SUS_JSON_TYPE_BOOLEAN:
		return a.value.boolean == b.value.boolean;
	case SUS_JSON_TYPE_STRING_VIEW:
	case SUS_JSON_TYPE_STRING:
		return a.value.str && b.value.str && lstrcmpA(a.value.str, b.value.str) == 0;
	case SUS_JSON_TYPE_ARRAY: {
		if (a.value.array->length != b.value.array->length) return FALSE;
		susVecForeach(i, a.value.array) if (!susJsonEquals(*susJsonArrayGet(a, i), *susJsonArrayGet(b, i))) return FALSE;
	} return TRUE;
	case SUS_JSON_TYPE_OBJECT: {
		if (a.value.object->count != b.value.object->count) return FALSE;
		susMapForeach(a.value.object, i) {
			SUS_LPJSON json = susJsonObjectGet(b, *(LPSTR*)susMapIterKey(i));
			if (!json) return FALSE;
			if (!susJsonEquals(*(SUS_JSON*)susMapIterValue(i), *json)) return FALSE;
		}
	} return TRUE;
	default:
		return TRUE;
	}
}

// -----------------------------------------------

// =======================================================================================

////////////////////////////////////////////////////////////////////////////////////////////////////
//								       Working with json objects                                  //
////////////////////////////////////////////////////////////////////////////////////////////////////

// =======================================================================================

// -----------------------------------------------

// Set a value for an object
SUS_LPJSON SUSAPI susJsonObjectSet(_Inout_ SUS_LPJSON obj, _In_ LPCSTR key, _In_ SUS_JSON value)
{
	SUS_ASSERT(obj && key);
	if (obj->type != SUS_JSON_TYPE_OBJECT || !obj->value.object) { susJsonDestroy(obj); *obj = susJsonObject(); }
	SUS_LPJSON json = susJsonObjectGet(*obj, key);
	if (json) susJsonDestroy(json);
	else {
		key = sus_strdup(key);
		json = susMapAdd(&obj->value.object, &key, NULL);
	}
	*json = susJsonCopy(value);
	return json;
}
// Delete an object value
VOID SUSAPI susJsonObjectRemove(_Inout_ SUS_LPJSON obj, _In_ LPCSTR key)
{
	SUS_ASSERT(obj && obj->type == SUS_JSON_TYPE_OBJECT && obj->value.object && key && susJsonObjectGet(*obj, key));
	SUS_OBJECT entry = susMapGetEntry(obj->value.object, &key);
	SUS_LPJSON value = susMapValue(obj->value.object, entry);
	susJsonDestroy(value);
	LPSTR keyCopy = *(LPSTR*)susMapKey(obj->value.object, entry);
	susMapRemove(&obj->value.object, &keyCopy);
	sus_strfree(keyCopy);
}

// -----------------------------------------------

// =======================================================================================
