#pragma once

// Warning: thread UNSAFE.

#include "pimplidiom.h"
#include "noncopyable.h"

template <class T>
class singleton : private NonCopyable {
public:
	virtual ~singleton() {}

public:
	static T* instance() {
		static T object;
		return &object;
	}
};

template <class T>
class singleton2 : public singleton<T>, public PimplIdiom {
public:
	singleton2(void* d, void(*destroyer)(void*)) : PimplIdiom(d, destroyer) {}
};
