#pragma once

// SUEDE TODO: Thread safe.
// uncomment these two line will take ZThread out....
//#include "../3rdparty/zthread-2.3.2/include/ZThread/Guard.h"
//#include "../3rdparty/zthread-2.3.2/include/ZThread/FastMutex.h"

#include "noncopyable.h"

#define __SUEDE_DEFINE_DESTROYER__(T, Name) \
template <class T> \
class Name { \
	T* doomed; \
public: \
	Destroyer(T* p = nullptr) : doomed(p) {} \
	~Destroyer() { reset(nullptr); } \
public: \
	void reset(T* p) { delete doomed; doomed = p; }\
}

template <class T>
class Singleton : private NonCopyable {
	static T* ptr;
	__SUEDE_DEFINE_DESTROYER__(T, Destroyer);

public:
	virtual ~Singleton() {}

public:
	static T* instance() {
		if (ptr == nullptr) {
			ptr = new T;
			static Destroyer<T> destroyer(ptr);
		}

		return ptr;
	}
};

template <class T>
T* Singleton<T>::ptr = nullptr;

template <class T>
class Singleton2 : private NonCopyable {
	static T* ptr;
	__SUEDE_DEFINE_DESTROYER__(T, Destroyer);

public:
	virtual ~Singleton2() {}

public:
	static T* instance() { return ptr; }
	static void implement(T* impl);
};

template <class T>
T* Singleton2<T>::ptr = nullptr;

template <class T>
void Singleton2<T>::implement(T * impl) {
	ptr = impl;
	static Destroyer<T> destroyer;
	destroyer.reset(ptr);
}
