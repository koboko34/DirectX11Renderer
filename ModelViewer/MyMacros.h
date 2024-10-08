#pragma once

#include <cassert>

#define FALSE_IF_FAILED(Call) Result = Call; assert(Result);
#define HFALSE_IF_FAILED(Call) hResult = Call; assert(!FAILED(hResult));
