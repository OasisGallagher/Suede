#pragma once
#include <ZThread/Runnable.h>

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
