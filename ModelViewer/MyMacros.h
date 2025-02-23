#pragma once

#include <cassert>

#define FALSE_IF_FAILED(Call) Result = Call; if (!Result) return false;
#define HFALSE_IF_FAILED(Call) hResult = Call; if (FAILED(hResult)) return false;
#define ASSERT_NOT_FAILED(Call) hResult = Call; assert(FAILED(hResult) == false);
