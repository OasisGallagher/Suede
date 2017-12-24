#pragma once

#ifdef  COMPILER_EXPORT
#define COMPILER_API __declspec(dllexport)
#else
#define COMPILER_API __declspec(dllimport)
#endif

// 标记的最大字符数.
#define COMPOLIER_MAX_TOKEN_CHARACTERS				48

// 一行的最大字符数.
#define COMPOLIER_MAX_LINE_CHARACTERS				512

// 产生式函数的最大参数个数.
#define COMPOLIER_MAX_PARSER_FUNCTION_PARAMTERS		8

typedef long long uint64;
