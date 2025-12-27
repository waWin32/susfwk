// coreframe.h
//
#pragma once

#define WIN32_LEAN_AND_MEAN
#define WIN32_EXTRA_LEAN
#define STRICT
#define NOSERVICE
#define NOMCX
#define NOIME
#define _WIN32_WINNT _WIN32_WINNT_WIN10
#define _WINSOCKAPI_

////////////////////////////////////////////////////////////////////////////////////////////////////

#include <Windows.h>
#include <TlHelp32.h>
#include <CommCtrl.h>
#include <wincrypt.h>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <winhttp.h>
#include <GL/glew.h>
#include <GL/wglew.h>

#define SUS_DEBUGONLYERRORS

#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "comctl32.lib")

