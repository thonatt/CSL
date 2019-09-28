#pragma once

#pragma warning( push )
//variadic macros with empty arguments
#pragma warning( disable : 4003)

#pragma warning( push )
//multiple assignment operators specified
#pragma warning( disable : 4522)


#include "Shaders.hpp"

#pragma warning( pop ) //4522
//#pragma warning( pop ) //4003 is not popped because it involves the preprocessor