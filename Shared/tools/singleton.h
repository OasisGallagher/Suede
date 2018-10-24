#pragma once

// Warning: thread UNSAFE.

#include "pimplidiom.h"
#include "noncopyable.h"

template <class T>
class Singleton : private NonCopyable {
	static T* ptr;

public:
	virtual ~Singleton() {}

public:
	static T* instance() {
		static T object;
		return &object;
	}
};

template <class T> T* Singleton<T>::ptr = nullptr;

template <class T>
class Singleton2 : public Singleton<T>, public PimplIdiom {
public:
	Singleton2(void* d, void(*destroyer)(void*)) : PimplIdiom(d, destroyer) {}
};
