#pragma once

#ifdef  COMPILER_EXPORT
#define COMPILER_API __declspec(dllexport)
#else
#define COMPILER_API __declspec(dllimport)
#endif

// һ�е�����ַ���.
#define COMPILER_MAX_LINE_CHARACTERS				512

// ����ʽ����������������.
#define COMPILER_MAX_PARSER_FUNCTION_PARAMTERS		8

#define COMPILER_GLPROGRAM							"GLPROGRAM"
#define COMPILER_ENDGL								"ENDGL"

typedef long long uint64;
