#pragma once
#include <ZThread/Guard.h>
#include <ZThread/Mutex.h>
#include <ZThread/Runnable.h>

template <class T>
class Mutex {
public:
	static ZThread::Mutex mutex;
};

template <class T>
ZThread::Mutex Mutex<T>::mutex;
#define GUARD_SCOPE(T)	ZThread::Guard<ZThread::Mutex> __guardScope(Mutex<T>::mutex)

class AsyncEventListener {
public:
	virtual void OnAsyncFinished(ZThread::Runnable* runnable) = 0;
};

class AsyncWorker : public ZThread::Runnable {
public:
	AsyncWorker(AsyncEventListener* listener) : listener_(listener) {}

public:
	// override OnRun instead.
	virtual void run();

protected:
	virtual void OnRun() = 0;

private:
	AsyncEventListener* listener_;
};
