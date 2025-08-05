// processapi.c
//
#include "coreframe.h"
#include "include/susfwk/core.h"
#include "include/susfwk/thrprocessapi.h"

// Global Critical Section
CRITICAL_SECTION g_cs;

// Create a snapshot of the system
SUS_FILE SUSAPI susCreateSystemSnapshot(_In_ DWORD dwFlags)
{
	SUS_PRINTDL("Creating a snapshot of the system");
	SUS_FILE hSnapShot = CreateToolhelp32Snapshot(dwFlags, 0);
	if (hSnapShot == INVALID_HANDLE_VALUE) {
		SUS_PRINTDE("Failed to create a snapshot of the system");
		SUS_PRINTDC(GetLastError());
		return NULL;
	}
	SUS_PRINTDL("A snapshot of the system has been created successfully!");
	return hSnapShot;
}
// Waiting for the thread to finish
BOOL SUSAPI susWaitForObjectFinish(_In_ SUS_FILE hObject)
{
	SUS_PRINTDL("Waiting for the end of the thread ...");
	SUS_ASSERT(hObject);
	if (WaitForSingleObject(hObject, INFINITE) == WAIT_FAILED) {
		SUS_PRINTDE("Couldn't wait for the thread to finish");
		SUS_PRINTDC(GetLastError());
		return (DWORD)-1;
	}
	SUS_PRINTDL("The thread has ended");
	DWORD exitCode;
	if (!GetExitCodeThread(hObject, &exitCode)) {
		GetExitCodeProcess(hObject, &exitCode);
	}
	return exitCode;
}

//////////////////////////////////////////////////////////////////
//						The Process API							//
//////////////////////////////////////////////////////////////////

// Open the process
SUS_PROCESS SUSAPI susOpenProcess(
	_In_ DWORD dwDesiredAccess,
	_In_ BOOLEAN bInheritHandle,
	_In_ DWORD dwProcessId)
{
	SUS_PRINTDL("Opening the process");
	SUS_ASSERT(dwProcessId != 0);
	SUS_PROCESS hProcess = OpenProcess(dwDesiredAccess, bInheritHandle, dwProcessId);
	if (!hProcess) {
		SUS_PRINTDE("Couldn't open the process");
		SUS_PRINTDC(GetLastError());
		return NULL;
	}
	SUS_PRINTDL("The process has been successfully opened!");
	return hProcess;
}

// Create a process
BOOL SUSAPI susCreateProcessA(
	_In_ LPCSTR lpApplicationNameCommandLine,
	_In_opt_ STARTUPINFOA *psi,
	_In_opt_ PROCESS_INFORMATION *ppi)
{
	SUS_PRINTDL("Creating a process");
	STARTUPINFOA si = { 0 };
	si.cb = sizeof(si);
	PROCESS_INFORMATION pi = { 0 };
	if (!CreateProcessA(
		NULL,
		(LPSTR)lpApplicationNameCommandLine,
		NULL, NULL,
		FALSE, 0,
		NULL, NULL,
		&si, &pi
	)) {
		SUS_PRINTDE("Failed to create a process");
		SUS_PRINTDC(GetLastError());
		return FALSE;
	}
	if (psi) *psi = si;
	if (ppi) *ppi = pi;
	else {
		CloseHandle(pi.hThread);
		CloseHandle(pi.hProcess);
	}
	SUS_PRINTDL("The process was created successfully");
	return TRUE;
}
// Create a process
BOOL SUSAPI susCreateProcessW(
	_In_ LPCWSTR lpApplicationNameCommandLine,
	_In_opt_ STARTUPINFOW* psi,
	_In_opt_ PROCESS_INFORMATION* ppi)
{
	SUS_PRINTDL("Creating a process");
	STARTUPINFOW si = { 0 };
	si.cb = sizeof(si);
	PROCESS_INFORMATION pi = { 0 };
	if (!CreateProcessW(
		NULL,
		(LPWSTR)lpApplicationNameCommandLine,
		NULL, NULL,
		FALSE, 0,
		NULL, NULL,
		&si, &pi
	)) {
		SUS_PRINTDE("Failed to create a process");
		SUS_PRINTDC(GetLastError());
		return FALSE;
	}
	if (psi) *psi = si;
	if (ppi) *ppi = pi;
	else {
		CloseHandle(pi.hThread);
		CloseHandle(pi.hProcess);
	}
	SUS_PRINTDL("The process was created successfully");
	return TRUE;
}

// --------------------------------------------------------------

// Get the first process
PROCESSENTRY32 SUSAPI susProcessFirstA(_In_ SUS_FILE hSnapShot)
{
	SUS_PRINTDL("Getting the first process");
	PROCESSENTRY32 pe32;
	pe32.dwSize = sizeof(PROCESSENTRY32);
	if (!Process32First(hSnapShot, &pe32)) {
		SUS_PRINTDE("Failed to get the first process");
		SUS_PRINTDC(GetLastError());
		return (PROCESSENTRY32) { 0 };
	}
	SUS_PRINTDL("The first process was successfully received!");
	return pe32;
}
// Get the first process
PROCESSENTRY32W SUSAPI susProcessFirstW(_In_ SUS_FILE hSnapShot)
{
	PROCESSENTRY32W pe32;
	pe32.dwSize = sizeof(PROCESSENTRY32W);
	if (!Process32FirstW(hSnapShot, &pe32)) {
		SUS_PRINTDE("Failed to get the first process");
		SUS_PRINTDC(GetLastError());
		return (PROCESSENTRY32W) { 0 };
	}
	return pe32;
}
// Go to the next process
BOOL SUSAPI susProcessNextA(_In_ SUS_FILE hSnapShot, _Inout_ LPPROCESSENTRY32 lpTe32)
{
	SUS_PRINTDL("Getting the next process");
	if (!Process32Next(hSnapShot, lpTe32)) {
		SUS_PRINTDL("Failed to get the next process");
		return FALSE;
	}
	SUS_PRINTDL("The next process was successfully received!");
	return TRUE;
}
// Go to the next process
BOOL SUSAPI susProcessNextW(_In_ SUS_FILE hSnapShot, _Inout_ LPPROCESSENTRY32W lpTe32)
{
	SUS_PRINTDL("Getting the next process");
	if (!Process32NextW(hSnapShot, lpTe32)) {
		SUS_PRINTDL("Failed to get the next process");
		return FALSE;
	}
	SUS_PRINTDL("The next process was successfully received!");
	return TRUE;
}
// Search process by its name
BOOL SUSAPI susFindProcessA(
	_In_ LPCSTR lpProcessName,
	_Out_opt_ LPPROCESSENTRY32 lpProcessEntry)
{
	SUS_PRINTDL("Search for a process by name");
	SUS_ASSERT(lpProcessName);
	if (lpProcessEntry) *lpProcessEntry = (PROCESSENTRY32){ 0 };
	SUS_FILE hSnapShot = susCreateProcessSnapshot();
	if (!hSnapShot) {
		SUS_PRINTDE("Couldn't find the process data");
		return FALSE;
	}
	PROCESSENTRY32 pe32 = susProcessFirstA(hSnapShot);
	if (!pe32.dwSize) {
		sus_fclose(hSnapShot);
		SUS_PRINTDE("Couldn't find the process data");
		return FALSE;
	}
	do {
		if (lstrcmpiA(pe32.szExeFile, lpProcessName) == 0) {
			sus_fclose(hSnapShot);
			if (lpProcessEntry) *lpProcessEntry = pe32;
			SUS_PRINTDL("The process %s found!", lpProcessName);
			return TRUE;
		}
	} while (susProcessNextA(hSnapShot, &pe32));
	sus_fclose(hSnapShot);
	SUS_PRINTDL("Process is not found");
	return FALSE;
}
// Search process by its name
BOOL SUSAPI susFindProcessW(
	_In_ LPCWSTR lpProcessName,
	_Out_opt_ LPPROCESSENTRY32W lpProcessEntry)
{
	SUS_PRINTDL("Search for a process by name");
	SUS_ASSERT(lpProcessName);
	if (lpProcessEntry) *lpProcessEntry = (PROCESSENTRY32W){ 0 };
	SUS_FILE hSnapShot = susCreateProcessSnapshot();
	if (!hSnapShot) {
		SUS_PRINTDE("Couldn't find the process data");
		return FALSE;
	}
	PROCESSENTRY32W pe32 = susProcessFirstW(hSnapShot);
	if (!pe32.dwSize) {
		sus_fclose(hSnapShot);
		SUS_PRINTDE("Couldn't find the process data");
		return FALSE;
	}
	do {
		if (lstrcmpiW(pe32.szExeFile, lpProcessName) == 0) {
			sus_fclose(hSnapShot);
			if (lpProcessEntry) *lpProcessEntry = pe32;
			SUS_PRINTDL("The process %s found!", lpProcessName);
			return TRUE;
		}
	} while (susProcessNextW(hSnapShot, &pe32));
	sus_fclose(hSnapShot);
	SUS_PRINTDL("Process is not found");
	return FALSE;
}

// --------------------------------------------------------------

//////////////////////////////////////////////////////////////////
//						The Thread API							//
//////////////////////////////////////////////////////////////////

// Open a thread
SUS_THREAD SUSAPI susOpenThread(
	_In_ DWORD dwDesiredAccess,
	_In_ BOOLEAN bInheritHandle,
	_In_ DWORD dwThreadId)
{
	SUS_PRINTDL("Opening a thread");
	SUS_THREAD hThr = (SUS_THREAD)OpenThread(dwDesiredAccess, bInheritHandle, dwThreadId);
	if (!hThr) {
		SUS_PRINTDE("Couldn't open the thread");
		SUS_PRINTDC(GetLastError());
		return NULL;
	}
	SUS_PRINTDL("The thread has been opened successfully");
	return hThr;
}
// Create a thread
SUS_THREAD SUSAPI susCreateThread(_In_ LPTHREAD_START_ROUTINE lpThrFunc, _In_opt_ LPVOID lpParam)
{
	SUS_PRINTDL("Creating a new thread");
	SUS_ASSERT(lpThrFunc);
	HANDLE hThr = CreateThread(NULL, 0, lpThrFunc, lpParam, 0, NULL);
	if (!hThr) {
		SUS_PRINTDE("Failed to create a new thread");
		SUS_PRINTDC(GetLastError());
		return NULL;
	}
	SUS_PRINTDL("The thread has been created successfully!");
	return (SUS_THREAD)hThr;
}
// Create a remote thread
SUS_THREAD SUSAPI susCreateRemoteThread(_In_ SUS_PROCESS hProcess, _In_ LPTHREAD_START_ROUTINE lpThrFunc, _In_opt_ LPVOID lpParam)
{
	SUS_PRINTDL("Create a new thread in another process");
	SUS_ASSERT(lpThrFunc);
	HANDLE hThr = CreateRemoteThread(hProcess, NULL, 0, lpThrFunc, lpParam, 0, NULL);
	if (!hThr) {
		SUS_PRINTDE("Failed to create a new thread");
		SUS_PRINTDC(GetLastError());
		return NULL;
	}
	SUS_PRINTDL("The thread has been created successfully!");
	return (SUS_THREAD)hThr;
}
// Terminate the thread
BOOL SUSAPI susTerminateThread(_In_ SUS_THREAD hThr)
{
	SUS_PRINTDL("Thread shutdown");
	SUS_ASSERT(hThr);
#pragma warning(push)
#pragma warning(disable : 4996)
	if (!TerminateThread(hThr, 0)) {
		SUS_PRINTDE("Failed to complete the flow");
		SUS_PRINTDC(GetLastError());
		return FALSE;
	}
#pragma warning(pop)
	sus_fclose(hThr);
	SUS_PRINTDL("The flow has been completed successfully");
	return TRUE;
}
// Set process priority
BOOL SUSAPI susSetThreadPriority(_In_ SUS_THREAD hThr, _In_ INT priority)
{
	SUS_PRINTDL("Setting the thread priority");
	SUS_ASSERT(hThr);
	if (!SetThreadPriority(hThr, priority)) {
		SUS_PRINTDE("Thread priority could not be set");
		SUS_PRINTDC(GetLastError());
		return FALSE;
	}
	SUS_PRINTDL("The thread has changed its priority");
	return TRUE;
}
// Suspend the thread operation
BOOL SUSAPI susSuspendThread(_In_ SUS_THREAD hThr)
{
	SUS_PRINTDL("Suspending the thread");
	SUS_ASSERT(hThr);
	if (SuspendThread(hThr) == -1) {
		SUS_PRINTDE("Couldn't suspend the stream");
		SUS_PRINTDC(GetLastError());
		return FALSE;
	}
	SUS_PRINTDL("The thread has been successfully suspended");
	return TRUE;
}
// Resume the thread operation
BOOL SUSAPI susResumeThread(_In_ SUS_THREAD hThr)
{
	SUS_PRINTDL("Resuming the flow");
	SUS_ASSERT(hThr);
	if (ResumeThread(hThr) == -1) {
		SUS_PRINTDE("The thread could not resume operation");
		SUS_PRINTDC(GetLastError());
		return FALSE;
	}
	SUS_PRINTDL("The thread has successfully resumed its work");
	return TRUE;
}

// -------------------------------------------------------------------

// Get the first thread
THREADENTRY32 SUSAPI susThreadFirst(_In_ SUS_FILE hSnapShot)
{
	SUS_PRINTDL("Getting the first thread");
	THREADENTRY32 te32;
	te32.dwSize = sizeof(THREADENTRY32);
	if (!Thread32First(hSnapShot, &te32)) {
		SUS_PRINTDE("Failed to get the first thread");
		SUS_PRINTDC(GetLastError());
		return (THREADENTRY32) { 0 };
	}
	SUS_PRINTDL("The first thread was successfully received!");
	return te32;
}
// Go to the next thread
BOOL SUSAPI susThreadNext(_In_ SUS_FILE hSnapShot, _Inout_ LPTHREADENTRY32 lpTe32)
{
	if (!Thread32Next(hSnapShot, lpTe32)) {
		SUS_PRINTDL("Failed to get the next thread");
		return FALSE;
	}
	return TRUE;
}

// -------------------------------------------------------------------

//////////////////////////////////////////////////////////////////
//						The Dynamic Library API					//
//////////////////////////////////////////////////////////////////

// Load a dynamic library
BOOL SUSAPI susLoadLibraryA(_In_ LPCSTR path)
{
	SUS_PRINTDL("Loading a dynamic library");
	if (!LoadLibraryA(path)) {
		SUS_PRINTDE("Couldn't load dynamic library");
		SUS_PRINTDC(GetLastError());
		return FALSE;
	}
	SUS_PRINTDL("The library has been uploaded successfully!");
	return TRUE;
}
// Load a dynamic library
BOOL SUSAPI susLoadLibraryW(_In_ LPCWSTR path)
{
	SUS_PRINTDL("Loading a dynamic library");
	if (!LoadLibraryW(path)) {
		SUS_PRINTDE("Couldn't load dynamic library");
		SUS_PRINTDC(GetLastError());
		return FALSE;
	}
	SUS_PRINTDL("The library has been uploaded successfully!");
	return TRUE;
}
