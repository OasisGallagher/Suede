#pragma once
#include <ZThread/Mutex.h>
#include <ZThread/Guard.h>

template <class T>
class Mutex {
public:
	static ZThread::Mutex mutex;
};

template <class T>
ZThread::Mutex Mutex<T>::mutex;
#define GUARD_SCOPE_TYPED(T)	ZThread::Guard<ZThread::Mutex> typedGuard(Mutex<T>::mutex)
