#pragma once
#include "types.h"

#ifdef  SHARED_EXPORT
#define SHARED_API __declspec(dllexport)
#else
#define SHARED_API __declspec(dllimport)
#endif
