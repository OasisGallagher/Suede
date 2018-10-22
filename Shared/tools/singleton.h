#pragma once

// SUEDE TODO: Thread safe.
// uncomment these two line will take ZThread out....
//#include "../3rdparty/zthread-2.3.2/include/ZThread/Guard.h"
//#include "../3rdparty/zthread-2.3.2/include/ZThread/FastMutex.h"

#include "pimplidiom.h"
#include "noncopyable.h"

template <class T>
class Singleton : private NonCopyable {
	static T* ptr;
	static bool destroyed;

	class Destroyer {
		T** pptr;
		bool* pdestroyed;

	public:
		Destroyer(T** pp, bool* pd) : pptr(pp), pdestroyed(pd) {}
		~Destroyer() { *pdestroyed = true; delete *pptr; *pptr = nullptr; }
	};

public:
	virtual ~Singleton() {}

public:
	static T* instance() {
		if (ptr == nullptr) {
			if (destroyed) {
				throw "instance has been destroyed";
			}

			ptr = new T;
			static Destroyer destroyer(&ptr, &destroyed);
		}

		return ptr;
	}
};

template <class T> T* Singleton<T>::ptr = nullptr;
template <class T> bool Singleton<T>::destroyed = false;

template <class T>
class Singleton2 : public Singleton<T>, public PimplIdiom {
public:
	Singleton2(void* d, void(*destroyer)(void*)) : PimplIdiom(d, destroyer) {}
};
