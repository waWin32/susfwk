/*++ BUILD Version: 0001    Increment this if a change has global effects

Copyright (c) Archon. All rights reserved.

Module Name:

    susframework.h

Abstract:

    The main header file for the standard library of utility sets,
    which provides the implementation of standard functions.

--*/

#ifndef _SUS_FRAMEWORK_
#define _SUS_FRAMEWORK_

/*
* SUSNOTMATH
* SUSNOTTMATH
* SUSNOTWINDOW
* SSUSINIMAL
* SUSNOTCONSOLEAPI
* SUSNOTSTRING
* SUSNOTECS
* SUSNOTTHRPROCESSAPI
* SUSNOTNETWORK
* SUSNOTHTTP
* SUSNOTJSON
* SUSNOTJNET
* SUSNOTAPPDATA
*/

#include "susfwk/core.h"
#include "susfwk/bitset.h"
#include "susfwk/string.h"
#include "susfwk/debug.h"
#include "susfwk/iostream.h"

#include "susfwk/memory.h"
#include "susfwk/buffer.h"
#include "susfwk/vector.h"
#include "susfwk/linkedlist.h"
#include "susfwk/hashtable.h"

#ifndef SSUSINIMAL
    #include "susfwk/regapi.h"
    #include "susfwk/resapi.h"
    #ifndef SUSNOTTHRPROCESSAPI
    #include "susfwk/thrprocessapi.h"
    #endif // !SUSNOTPROCESSAPI
    #ifndef SUSNOTWINDOW
    #include "susfwk/window.h"
    #endif // !SUSNOTWINDOW
    #ifndef SUSNOTMATH
    #include "susfwk/math.h"
    #ifndef SUSNOTTMATH
    #include "susfwk/tmath.h"
    #endif // !SUSNOTTMATH
    #endif // !SUSNOTMATH
    #ifndef SUSNOTECS
    #include "susfwk/ecs.h"
    #endif // !SUSNOTECS
    #ifndef SUSNOTNETWORK
    #include <WinSock2.h>
    #include <WS2tcpip.h>
    #include "susfwk/network.h"
    #ifndef SUSNOTJNET
    #include "susfwk/jnet.h"
    #endif // !SUSNOTJNET
    #endif // !SUSNOTNETWORK
    #ifndef SUSNOTHTTP
    #include <winhttp.h>
    #include "susfwk/httprequest.h"
    #endif // !SUSNOTHTTP
    #ifndef SUSNOTJSON
    #include "susfwk/json.h"
    #endif // !SUSNOTJSON
    #ifndef SUSNOTAPPDATA
    #include "susfwk/appdata.h"
    #endif // !SUSNOTAPPDATA
#endif // !SSUSINIMAL

#endif /* !_SUS_FRAMEWORK_ */