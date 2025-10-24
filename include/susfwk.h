
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
* SUSNOTWINDOW
* SSUSINIMAL
* SUSNOTCONSOLEAPI
* SUSNOTSTRING
* SUSNOTECS
* SUSNOTCRYPTO
* SUSNOTTHRPROCESSAPI
* SUSNOTNETWORK
* SUSNOTHTTP
* SUSNOTJSON
* SUSNOTJNET
*/

#include "susfwk/core.h"
#include "susfwk/bitset.h"
#include "susfwk/string.h"
#include "susfwk/debug.h"
#include "susfwk/iostream.h"

#include "susfwk/memory.h"
#include "susfwk/vector.h"
#include "susfwk/linkedlist.h"
#include "susfwk/hashtable.h"

#ifndef SSUSINIMAL
    #include "susfwk/regapi.h"
    #ifndef SUSNOTCRYPTO
    #include <wincrypt.h>
    #include "susfwk/crypto.h"
    #endif // !SUSNOTCRYPTO
    #include "susfwk/resapi.h"
    #ifndef SUSNOTTHRPROCESSAPI
    #include "susfwk/thrprocessapi.h"
    #endif // !SUSNOTPROCESSAPI
    #ifndef SUSNOTWINDOW
    #include "susfwk/windowapi.h"
    #endif // !SUSNOTWINDOW
    #ifndef SUSNOTMATH
    #include "susfwk/math.h"
    #endif // !SUSNOTMATH
    #ifndef SUSNOTECS
    #include "susfwk/ecs.h"
    #endif // !SUSNOTECS
    #ifndef SUSNOTNETWORK
    #include <WinSock2.h>
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
#endif // !SSUSINIMAL

#endif /* !_SUS_FRAMEWORK_ */