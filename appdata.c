// appdata.c
//
#include "coreframe.h"
#include "include/susfwk/core.h"
#include "include/susfwk/thrprocessapi.h"
#include "include/susfwk/hashtable.h"
#include "include/susfwk/appdata.h"

// Application Data
typedef struct sus_appdata {
	SUS_HASHMAP data;	// LPCSTR -> SUS_OBJECT
	SUS_MUTEX	mutex;	// App mutex
} SUS_APPDATA, *SUS_LPAPPDATA;

static SUS_APPDATA appData = { 0 };

// Initialize application data
VOID SUSAPI susAppInit()
{
	SUS_PRINTDL("Initializing application data");
	SUS_ASSERT(!appData.data);
	appData.mutex = susMutexSetup();
	appData.data = susNewMap(LPCSTR, SUS_OBJECT);
}
// Install the application data
SUS_OBJECT SUSAPI susAppSet(_In_ LPCSTR key, _In_ SUS_OBJECT value)
{
	SUS_PRINTDL("Application data installation - \"%s\"", key);
	SUS_ASSERT(appData.data && key);
	susMutexLock(&appData.mutex);
	value = *(SUS_OBJECT*)susMapSet(&appData.data, &key, &value);
	susMutexUnlock(&appData.mutex);
	return value;
}
// Get application data
SUS_OBJECT SUSAPI susAppGet(_In_ LPCSTR key)
{
	SUS_PRINTDL("Getting application data - \"%s\"", key);
	SUS_ASSERT(appData.data && key);
	return *(SUS_OBJECT*)susMapGet(appData.data, &key);
}
// Get application data
SUS_LPMUTEX SUSAPI susAppGetMutex()
{
	SUS_PRINTDL("Getting the mutex application");
	SUS_ASSERT(appData.data);
	return &appData.mutex;
}
// Clean the application data
VOID SUSAPI susAppCleanup()
{
	SUS_PRINTDL("Clearing application data");
	SUS_ASSERT(appData.data);
	susMapDestroy(appData.data);
	susMutexCleanup(&appData.mutex);
	appData.data = NULL;
}
