// appdata.h
//
#ifndef _SUS_APP_DATA_
#define _SUS_APP_DATA_

// -----------------------------------------------

//////////////////////////////////////////////
// KeyFormat - "/module/submodule/.../name"	//
// Key - String literal						//
// Value - Pointer							//
//////////////////////////////////////////////

// -----------------------------------------------

// Initialize application data
VOID SUSAPI susAppInit();
// Clean the application data
VOID SUSAPI susAppCleanup();

// -----------------------------------------------

// Install the application data
SUS_OBJECT SUSAPI susAppSet(_In_ LPCSTR key, _In_ SUS_OBJECT value);
// Get application data
SUS_OBJECT SUSAPI susAppGet(_In_ LPCSTR key);
// Get your information about the app
VOID SUSAPI susAppSetData(LONG_PTR data);
// Get your information about the app
LONG_PTR SUSAPI susAppGetData();
// Get application data
SUS_LPMUTEX SUSAPI susAppGetMutex();

// -----------------------------------------------

#endif // !_SUS_APP_DATA_
