#pragma once
#include <memory>

#ifdef __EXPORT_ENGINE__
#define ENGINE_EXPORT __declspec(dllexport)
#else
#define ENGINE_EXPORT __declspec(dllimport)
#endif

struct Deleter {
	void operator()(int *p) {
		delete p;
	}
};

template <class T>
struct SharedPtr {
	typedef std::shared_ptr<T> T;
};

#define DEFINE_OBJECT_PTR(Type)	typedef SharedPtr<class I ## Type>::T Type
