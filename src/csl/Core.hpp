#pragma once

#pragma warning( push )
//variadic macros with empty arguments
#pragma warning( disable : 4003)

#pragma warning( push )
//multiple assignment operators specified
#pragma warning( disable : 4522)

//to define "not" keyword
#ifdef _WIN32	
#include <iso646.h>
#endif

#define CSL_VERSION_MAJOR 0
#define CSL_VERSION_MINOR 1

#define CSL_MAX_MINOR_VERSION 1000

#define CSL_VERSION (CSL_VERSION_MAJOR * CSL_MAX_MINOR_VERSION + CSL_VERSION_MINOR)

#include "Shaders.hpp"

#pragma warning( pop ) //4522
//#pragma warning( pop ) //4003 is not popped because it involves the preprocessor