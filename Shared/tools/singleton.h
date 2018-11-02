#pragma once

// Warning: thread UNSAFE.

#include "pimplidiom.h"
#include "noncopyable.h"

template <class T>
class Singleton : private NonCopyable {
public:
	virtual ~Singleton() {}

public:
	static T* instance() {
		static T object;
		return &object;
	}
};

template <class T>
class Singleton2 : public Singleton<T>, public PimplIdiom {
public:
	Singleton2(void* d, void(*destroyer)(void*)) : PimplIdiom(d, destroyer) {}
};
