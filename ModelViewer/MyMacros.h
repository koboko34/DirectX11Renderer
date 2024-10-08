#pragma once

#include <cassert>

#define FALSE_IF_FAILED(Call) Result = Call; if (!Result) return false;
#define HFALSE_IF_FAILED(Call) hResult = Call; if (FAILED(hResult)) return false;
