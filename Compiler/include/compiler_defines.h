#pragma once

#ifdef  COMPILER_EXPORT
#define COMPILER_API __declspec(dllexport)
#else
#define COMPILER_API __declspec(dllimport)
#endif

// ��ǵ�����ַ���.
#define COMPOLIER_MAX_TOKEN_CHARACTERS				48

// һ�е�����ַ���.
#define COMPOLIER_MAX_LINE_CHARACTERS				512

// ����ʽ����������������.
#define COMPOLIER_MAX_PARSER_FUNCTION_PARAMTERS		8

typedef long long uint64;
