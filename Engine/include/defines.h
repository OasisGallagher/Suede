#pragma once
#include <memory>

#ifdef __EXPORT_ENGINE__
#define ENGINE_EXPORT __declspec(dllexport)
#else
#define ENGINE_EXPORT __declspec(dllimport)
#endif

#define DEFINE_OBJECT_PTR(T)	typedef std::shared_ptr<class I ## T> T
