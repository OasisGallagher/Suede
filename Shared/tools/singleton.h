#pragma once

// SUEDE TODO: Thread safe.
// uncomment these two line will take ZThread out....
//#include "../3rdparty/zthread-2.3.2/include/ZThread/Guard.h"
//#include "../3rdparty/zthread-2.3.2/include/ZThread/FastMutex.h"

#include "noncopyable.h"

template <class T>
class Singleton : private NonCopyable {
	static T* ptr;
	class Destroyer {
		T* doomed;
	public:
		Destroyer(T* p = nullptr) : doomed(p) {}
		~Destroyer() { reset(nullptr); }
	public:
		void reset(T* p) { delete doomed; doomed = p; }
	};

public:
	virtual ~Singleton() {}

public:
	static T* instance() {
		if (ptr == nullptr) {
			ptr = new T;
			static Destroyer destroyer(ptr);
		}

		return ptr;
	}
};

template <class T>
T* Singleton<T>::ptr = nullptr;

template <class T>
class Singleton2 : public Singleton<T> {
protected:
	void *d_;

public:
	Singleton2(void* d) : d_(d) {}
};
