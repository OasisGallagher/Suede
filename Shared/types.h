#pragma once

typedef unsigned int uint;
typedef unsigned char uchar;
typedef unsigned long ulong;

typedef long long int64;
typedef unsigned long long uint64;

typedef uint suede_guid;

#ifdef SUEDE_EXPORT
#define SUEDE_API __declspec(dllexport)
#else
#define SUEDE_API __declspec(dllimport)
#endif

#ifdef SUEDE_MATH_EXPORT
#define SUEDE_MATH_API __declspec(dllexport)
#else
#define SUEDE_MATH_API __declspec(dllimport)
#endif
