#pragma once

#ifdef  COMPILER_EXPORT
#define COMPILER_API __declspec(dllexport)
#else
#define COMPILER_API __declspec(dllimport)
#endif

#define COMPOLIER_ITEMSET_NAME_SEPERATOR			'.'

// ��ǵ�����ַ���.
#define COMPOLIER_MAX_TOKEN_CHARACTERS				32

// һ�е�����ַ���.
#define COMPOLIER_MAX_LINE_CHARACTERS				512

// ����ʽ����������������.
#define COMPOLIER_MAX_PARSER_FUNCTION_PARAMTERS		4

typedef long long uint64;
