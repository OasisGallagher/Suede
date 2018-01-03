#pragma once
#include <memory>
#include "types.h"

#ifdef  SUEDE_EXPORT
#define SUEDE_API __declspec(dllexport)
#else
#define SUEDE_API __declspec(dllimport)
#endif

#define SUEDE_DEFINE_OBJECT_POINTER(T)	typedef std::shared_ptr<class I ## T> T
#define SUEDE_DECLARE_OBJECT_CREATER(T)	SUEDE_API T New ## T()

#define SUEDE_USE_NAMESPACE

#ifdef  SUEDE_USE_NAMESPACE
#define SUEDE_BEGIN_NAMESPACE	namespace {
#define SUEDE_END_NAMESPACE		}
#else
#define SUEDE_BEGIN_NAMESPACE
#define SUEDE_END_NAMESPACE
#endif