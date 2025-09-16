// processapi.h
//
#ifndef _SUS_PROCESS_API_
#define _SUS_PROCESS_API_

#include <TlHelp32.h>

// Create a snapshot of the system
SUS_FILE SUSAPI susCreateSystemSnapshot(_In_ DWORD dwFlags);
// Waiting for the thread to finish
BOOL SUSAPI susWaitForObjectFinish(_In_ SUS_FILE hObject);

//////////////////////////////////////////////////////////////////
//						The Process API							//
//////////////////////////////////////////////////////////////////

// Thread Descriptor
typedef SUS_FILE SUS_PROCESS;

// Open the process
SUS_PROCESS SUSAPI susOpenProcess(
	_In_ DWORD dwDesiredAccess,
	_In_ BOOLEAN bInheritHandle,
	_In_ DWORD dwProcessId
);
// Create a process
BOOL SUSAPI susCreateProcessA(
	_In_ LPCSTR lpApplicationNameCommandLine,
	_In_opt_ STARTUPINFOA* psi,
	_In_opt_ PROCESS_INFORMATION* ppi
);
// A quick way to create a process
#define susQuickCreateProcessA(lpApplicationNameCommandLine) susCreateProcessA(lpApplicationNameCommandLine, NULL, NULL)
// Create a process
BOOL SUSAPI susCreateProcessW(
	_In_ LPCWSTR lpApplicationNameCommandLine,
	_In_opt_ STARTUPINFOW* psi,
	_In_opt_ PROCESS_INFORMATION* ppi
);
// A quick way to create a process
#define susQuickCreateProcessW(lpApplicationNameCommandLine) susCreateProcessW(lpApplicationNameCommandLine, NULL, NULL)

#ifdef UNICODE
#define susQuickCreateProcess	susQuickCreateProcessW
#define susCreateProcess		susCreateProcessW
#else
#define susQuickCreateProcess	susQuickCreateProcessA
#define susCreateProcess		susCreateProcessA
#endif // !UNICODE

// --------------------------------------------------------------

// Create a snapshot of the system
SUS_INLINE SUS_FILE SUSAPI susCreateProcessSnapshot() { return susCreateSystemSnapshot(TH32CS_SNAPPROCESS); }

// Get the first process
PROCESSENTRY32 SUSAPI susProcessFirstA(
	_In_ SUS_FILE hSnapShot
);
// Get the first process
PROCESSENTRY32W SUSAPI susProcessFirstW(
	_In_ SUS_FILE hSnapShot
);

#ifdef UNICODE
#define susProcessFirst	susProcessFirstW
#else
#define susProcessFirst	susProcessFirstA
#endif // !UNICODE

// Go to the next process
BOOL SUSAPI susProcessNextA(_In_ SUS_FILE hSnapShot, _Inout_ LPPROCESSENTRY32 lpTe32);
// Go to the next process
BOOL SUSAPI susProcessNextW(_In_ SUS_FILE hSnapShot, _Inout_ LPPROCESSENTRY32W lpTe32);

#ifdef UNICODE
#define susProcessNext	susProcessNextW
#else
#define susProcessNext	susProcessNextA
#endif // !UNICODE

// Search process by its name
BOOL SUSAPI susFindProcessA(
	_In_ LPCSTR lpProcessName,
	_Out_opt_ LPPROCESSENTRY32 lpProcessEntry
);
// Search process by its name
BOOL SUSAPI susFindProcessW(
	_In_ LPCWSTR lpProcessName,
	_Out_opt_ LPPROCESSENTRY32W lpProcessEntry
);

#ifdef UNICODE
#define susFindProcess	susFindProcessW
#else
#define susFindProcess	susFindProcessA
#endif // !UNICODE

// --------------------------------------------------------------

//////////////////////////////////////////////////////////////////
//						The Thread API							//
//////////////////////////////////////////////////////////////////

// Thread Descriptor
typedef SUS_FILE SUS_THREAD;

// Open a thread
SUS_THREAD SUSAPI susOpenThread(
	_In_ DWORD dwDesiredAccess,
	_In_ BOOLEAN bInheritHandle,
	_In_ DWORD dwThreadId
);
// Create a thread
SUS_THREAD SUSAPI susCreateThread(
	_In_ LPTHREAD_START_ROUTINE lpThrFunc,
	_In_opt_ LPVOID lpParam,
	_In_ BOOL start
);
// Create a remote thread
SUS_THREAD SUSAPI susCreateRemoteThread(
	_In_ SUS_PROCESS hProcess, 
	_In_ LPTHREAD_START_ROUTINE lpThrFunc,
	_In_opt_ LPVOID lpParam,
	_In_ BOOL start
);
// Terminate the thread
BOOL SUSAPI susTerminateThread(
	_In_ SUS_THREAD hThr
);
// Set process priority
BOOL SUSAPI susSetThreadPriority(
	_In_ SUS_THREAD hThr,
	_In_ INT priority
);
// Suspend the thread operation
BOOL SUSAPI susSuspendThread(
	_In_ SUS_THREAD hThr
);
// Resume the thread operation
BOOL SUSAPI susResumeThread(
	_In_ SUS_THREAD hThr
);

// -------------------------------------------------------------------

// Create a snapshot of the system
SUS_INLINE SUS_FILE SUSAPI susCreateThreadSnapshot() { return susCreateSystemSnapshot(TH32CS_SNAPTHREAD); }
// Get the first thread
THREADENTRY32 SUSAPI susThreadFirst(_In_ SUS_FILE hSnapShot);
// Go to the next thread
BOOL SUSAPI susThreadNext(_In_ SUS_FILE hSnapShot, _Inout_ LPTHREADENTRY32 lpTe32);

// -------------------------------------------------------------------

//////////////////////////////////////////////////////////////////
//						The Dynamic Library API					//
//////////////////////////////////////////////////////////////////

// Load a dynamic library
BOOL SUSAPI susLoadLibraryA(_In_ LPCSTR path);
// Load a dynamic library
BOOL SUSAPI susLoadLibraryW(_In_ LPCWSTR path);

#endif /* !_SUS_PROCESS_API_ */
