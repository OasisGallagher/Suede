#pragma once

// TODO: Thead safe.
// uncomment these two line will bring ZThread out....
//#include "../3rdparty/zthread-2.3.2/include/ZThread/Guard.h"
//#include "../3rdparty/zthread-2.3.2/include/ZThread/FastMutex.h"

#include "noncopyable.h"

template <class T>// class LockType = ZThread::FastMutex>
class Singleton : private NonCopyable {
public:
	static T* get();

private:
	//! SingletonDestroyer
	template <class U>
	class Destroyer {
		U* doomed;

	public:
		Destroyer(U* q) : doomed(q) { }
		~Destroyer();
	};
};

template <class T> // class LockType>
T* Singleton<T/*LockType*/>::get() {
	// Uses local static storage to avoid static construction
	// sequence issues. (regaring when the lock is created)
	static T* ptr = nullptr;
	//static LockType lock;

	if (ptr == nullptr) {
		//ZThread::Guard<LockType, ZThread::LockedScope> g(lock);
		//if (!ptr)
		/*InstantiationPolicy::create(ptr);*/
		ptr = new T;
		static Destroyer<T> destroyer(ptr);
	}

	return ptr;
}

template <class T> template <class U>
Singleton<T>::Destroyer<U>::~Destroyer() {
	try {
		delete doomed;
	}
	catch (...) {
	}

	doomed = nullptr;
}
