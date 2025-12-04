// json.h
//
#ifndef _SUS_JSON_API_
#define _SUS_JSON_API_

#include "vector.h"
#include "hashtable.h"

////////////////////////////////////////////////////////////////////////////////////////////////////
//						       The structure of the json object                                   //
////////////////////////////////////////////////////////////////////////////////////////////////////

// =======================================================================================

// -----------------------------------------------

// Json type
typedef enum sus_json_type {
	SUS_JSON_TYPE_NULL,
	SUS_JSON_TYPE_NUMBER,
	SUS_JSON_TYPE_STRING,
	SUS_JSON_TYPE_STRING_VIEW,
	SUS_JSON_TYPE_BOOLEAN,
	SUS_JSON_TYPE_ARRAY,
	SUS_JSON_TYPE_OBJECT
} SUS_JSON_TYPE;
// The structure of the json object
typedef struct sus_json {
	SUS_JSON_TYPE type;		// Json type
	union {
		sus_str str;
		sus_f32 number;
		sus_bool boolean;
		SUS_VECTOR array; // SUS_JSON
		SUS_HASHMAP object; // LPSTR -> SUS_JSON
	} value;
} SUS_JSON, *SUS_LPJSON;
// Json return errors
typedef enum sus_json_error {
	SUS_JSON_ERROR_SUCCESS,
	SUS_JSON_ERROR_SYNTAX,
	SUS_JSON_ERROR_SYSTEM,
} SUS_JSON_ERROR, *SUS_LPJSON_ERROR;

// -----------------------------------------------

// =======================================================================================

////////////////////////////////////////////////////////////////////////////////////////////////////
//									 Creating json objects                                        //
////////////////////////////////////////////////////////////////////////////////////////////////////

// =======================================================================================

// -----------------------------------------------

// Create an empty json file
#define susJsonNull() (SUS_JSON) { 0 }
// Create numeric json
SUS_INLINE SUS_JSON SUSAPI susJsonNumber(_In_ FLOAT number) {
	return (SUS_JSON) {
		.type = SUS_JSON_TYPE_NUMBER,
		.value.number = number
	};
}
// Create string json
SUS_INLINE SUS_JSON SUSAPI susJsonString(_In_opt_ LPCSTR text) {
	return text ? (SUS_JSON) {
		.type = SUS_JSON_TYPE_STRING,
		.value.str = sus_strdup(text)
	} : susJsonNull();
}
// Create string json
SUS_INLINE SUS_JSON SUSAPI susJsonStringView(_In_opt_ LPCSTR text) {
	return text ? (SUS_JSON) {
		.type = SUS_JSON_TYPE_STRING_VIEW,
		.value.str = (LPSTR)text
	} : susJsonNull();
}
// Create Boolean json
SUS_INLINE SUS_JSON SUSAPI susJsonBoolean(_In_ BOOL boolean) {
	return (SUS_JSON) {
		.type = SUS_JSON_TYPE_BOOLEAN,
		.value.boolean = boolean
	};
}
// Create a json array
SUS_INLINE SUS_JSON SUSAPI susJsonArray() {
	return (SUS_JSON) {
		.type = SUS_JSON_TYPE_ARRAY,
		.value.array = susNewVector(SUS_JSON)
	};
}
// Create a json object
SUS_INLINE SUS_JSON SUSAPI susJsonObject() {
	return (SUS_JSON) {
		.type = SUS_JSON_TYPE_OBJECT,
		.value.object = susNewMapEx(sizeof(LPSTR), sizeof(SUS_JSON), susDefGetStringHashA, susDefCmpStringKeysA, 0)
	};
}

// -----------------------------------------------

// =======================================================================================

////////////////////////////////////////////////////////////////////////////////////////////////////
//								       Working with json objects                                  //
////////////////////////////////////////////////////////////////////////////////////////////////////

// =======================================================================================

// -----------------------------------------------

// Delete a json object
VOID SUSAPI susJsonDestroy(
	_Inout_ SUS_LPJSON json
);
// Deep copying of a json object
SUS_JSON SUSAPI susJsonCopy(
	_In_ SUS_JSON json
);
// Convert json to string
LPSTR SUSAPI susJsonStringify(
	_In_ SUS_JSON json
);
// Convert string to json
SUS_JSON SUSAPI susJsonParse(
	_In_ LPCSTR text,
	_Out_opt_ SUS_LPJSON_ERROR lpError
);

// -----------------------------------------------

// Comparison of 2 json objects
BOOL SUSAPI susJsonEquals(
	_In_ SUS_JSON a,
	_In_ SUS_JSON b
);
// Check whether the json is valid
SUS_INLINE BOOL SUSAPI susJsonIsValid(_In_ SUS_JSON json) {
	return json.type || ((json.type == SUS_JSON_TYPE_OBJECT || json.type == SUS_JSON_TYPE_ARRAY) && json.value.object);
}

// -----------------------------------------------

// =======================================================================================

////////////////////////////////////////////////////////////////////////////////////////////////////
//							    Functions for working with objects                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

// =======================================================================================

// -----------------------------------------------

// Get the object value
SUS_INLINE SUS_LPJSON SUSAPI susJsonObjectGet(_In_ SUS_JSON obj, _In_ LPCSTR key) {
	SUS_ASSERT(key);
	if (obj.type != SUS_JSON_TYPE_OBJECT || !obj.value.object) return NULL;
	return (SUS_LPJSON)susMapGet(obj.value.object, &key);
}
// Check the presence of an element in an object
SUS_INLINE BOOL SUSAPI susJsonObjectContains(_In_ SUS_JSON obj, _In_ LPCSTR key) {
	SUS_ASSERT(key);
	return susJsonObjectGet(obj, key) ? TRUE : FALSE;
}
// Get the number of items in an object
SUS_INLINE UINT SUSAPI susJsonObjectCount(_In_ SUS_JSON obj) {
	return obj.value.object ? obj.value.object->count : 0;
}
// Set a value for an object
SUS_LPJSON SUSAPI susJsonObjectSet(
	_Inout_ SUS_LPJSON obj,
	_In_ LPCSTR key,
	_In_ SUS_JSON value
);
// Delete an object value
VOID SUSAPI susJsonObjectRemove(
	_Inout_ SUS_LPJSON obj,
	_In_ LPCSTR key
);
//
#define susJsonObjectForeach(jsonObject, i) susMapForeach((jsonObject).value.object, i)

// -----------------------------------------------

// =======================================================================================

// -----------------------------------------------

// Get the length of a json array
SUS_INLINE DWORD SUSAPI susJsonArrayLength(_In_ SUS_JSON arr) {
	return arr.value.array ? arr.value.array->length : 0;
}
// Get a json array element
SUS_INLINE SUS_LPJSON SUSAPI susJsonArrayGet(_In_ SUS_JSON arr, _In_ DWORD i) {
	if (arr.type != SUS_JSON_TYPE_ARRAY || !arr.value.array) return NULL;
	return (SUS_LPJSON)susVectorGet(arr.value.array, i);
}
// Get a json array element
SUS_INLINE SUS_LPJSON SUSAPI susJsonArrayAt(_In_ SUS_JSON arr, _In_ INT i) {
	if (arr.type != SUS_JSON_TYPE_ARRAY || !arr.value.array) return NULL;
	return (SUS_LPJSON)susVectorAt(arr.value.array, i);
}
// Check if the element is contained
SUS_INLINE BOOL SUSAPI susJsonArrayContains(_In_ SUS_JSON arr, _In_ DWORD i) {
	if (arr.type != SUS_JSON_TYPE_ARRAY || !arr.value.array) return FALSE;
	return (SUS_LPJSON)susVectorGet(arr.value.array, i) ? TRUE : FALSE;
}

// -----------------------------------------------

// Add a value to the end of a json array
SUS_INLINE SUS_LPJSON SUSAPI susJsonArrayPush(_Inout_ SUS_LPJSON arr, _In_ SUS_JSON value) {
	SUS_ASSERT(arr);
	if (arr->type != SUS_JSON_TYPE_ARRAY || !arr->value.array) { susJsonDestroy(arr); *arr = susJsonArray(); }
	SUS_JSON json = susJsonCopy(value);
	return (SUS_LPJSON)susVectorPushBack(&arr->value.array, &json);
}
// Delete a value from the end of a json array
SUS_INLINE VOID SUSAPI susJsonArrayPop(_Inout_ SUS_LPJSON arr) {
	SUS_ASSERT(arr && arr->type == SUS_JSON_TYPE_ARRAY && arr->value.array && arr->value.array->length);
	susJsonDestroy((SUS_LPJSON)susVectorBack(arr->value.array));
	susVectorPopBack(&arr->value.array);
}

// Add a value to the beginning of the json array
SUS_INLINE SUS_LPJSON SUSAPI susJsonArrayUnshift(_Inout_ SUS_LPJSON arr, _In_ SUS_JSON value) {
	SUS_ASSERT(arr);
	if (arr->type != SUS_JSON_TYPE_ARRAY || !arr->value.array) { susJsonDestroy(arr); *arr = susJsonArray(); }
	SUS_JSON json = susJsonCopy(value);
	return (SUS_LPJSON)susVectorPushFront(&arr->value.array, &json);
}
// Delete a value from the beginning of a json array
SUS_INLINE VOID SUSAPI susJsonArrayShift(_Inout_ SUS_LPJSON arr) {
	SUS_ASSERT(arr && arr->type == SUS_JSON_TYPE_ARRAY && arr->value.array);
	susJsonDestroy((SUS_LPJSON)susVectorFront(arr->value.array));
	susVectorPopFront(&arr->value.array);
}

// Insert json at an arbitrary location in an array
SUS_INLINE SUS_LPJSON SUSAPI susJsonArrayInsert(_Inout_ SUS_LPJSON arr, _In_ SUS_JSON value, _In_ DWORD i) {
	SUS_ASSERT(arr);
	if (arr->type != SUS_JSON_TYPE_ARRAY || !arr->value.array) { susJsonDestroy(arr); *arr = susJsonArray(); }
	SUS_JSON json = susJsonCopy(value);
	return (SUS_LPJSON)susVectorInsert(&arr->value.array, i, &json);
}
// Remove json from an arbitrary location in an array
SUS_INLINE VOID SUSAPI susJsonArrayRemove(_Inout_ SUS_LPJSON arr, _In_ DWORD i) {
	SUS_ASSERT(arr && arr->type == SUS_JSON_TYPE_ARRAY && arr->value.array && i < arr->value.array->length);
	susJsonDestroy((SUS_LPJSON)susJsonArrayGet(*arr, i));
	susVectorErase(&arr->value.array, i);
}
// Replace json in the array
SUS_INLINE SUS_LPJSON SUSAPI susJsonArrayReplace(_Inout_ SUS_LPJSON arr, _In_ DWORD i, _In_ SUS_JSON value) {
	SUS_ASSERT(arr && arr->type == SUS_JSON_TYPE_ARRAY && arr->value.array);
	susJsonDestroy((SUS_LPJSON)susJsonArrayGet(*arr, i));
	SUS_JSON json = susJsonCopy(value);
	return (SUS_LPJSON)susVectorReplace(&arr->value.array, i, &json);
}

// -----------------------------------------------

// The function of comparing items in the search
static BOOL SUSAPI susJsonElementsCompareCallBack(_In_ SUS_LPJSON obj, _In_ SUS_LPJSON sought, _In_ SIZE_T size) {
	UNREFERENCED_PARAMETER(size);
	return susJsonEquals(*obj, *sought);
}
// Find the first item you see
SUS_INLINE INT SUSAPI susJsonArrayFind(_Inout_ SUS_JSON arr, _In_ SUS_JSON value) {
	if (arr.type != SUS_JSON_TYPE_ARRAY || !arr.value.array) return -1;
	return susVectorIndexOf(arr.value.array, &value, susJsonElementsCompareCallBack);
}
// Find the first available element from the end
SUS_INLINE INT SUSAPI susJsonArrayFindLast(_Inout_ SUS_JSON arr, _In_ SUS_JSON value) {
	if (arr.type != SUS_JSON_TYPE_ARRAY || !arr.value.array) return -1;
	return susVectorLastIndexOf(arr.value.array, &value, susJsonElementsCompareCallBack);
}
//
#define susJsonArrayForeach(jsonArray, i) susVecForeach(i, (jsonArray).value.array)

// -----------------------------------------------

// =======================================================================================

#endif // !_SUS_JSON_API_
