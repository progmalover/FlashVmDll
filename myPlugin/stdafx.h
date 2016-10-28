// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include "targetver.h"
#include <windows.h>
#endif

#include <string.h>
#include <stdlib.h>
#include <string>
 
using namespace std;

#include "../interface/resourceinterface.h"
#include "../cppsrc/writeloglineex.h"
#include "../cppsrc/oshelper.h"
