#pragma once

typedef unsigned int uint;
typedef unsigned char uchar;
typedef unsigned long ulong;
typedef unsigned long long uint64;

#ifdef SUEDE_EXPORT
#define SUEDE_API __declspec(dllexport)
#else
#define SUEDE_API __declspec(dllimport)
#endif
