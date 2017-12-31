#pragma once

#ifdef  GLEF_EXPORT
#define GLEF_API __declspec(dllexport)
#else
#define GLEF_API __declspec(dllimport)
#endif

#define GLEF_MAX_LINE_CHARACTERS				512
#define GLEF_MAX_PARSER_FUNCTION_PARAMTERS		8

#define GLSL_CODE_BEGIN							"GLSLPROGRAM"
#define GLSL_CODE_END							"ENDGLSL"
