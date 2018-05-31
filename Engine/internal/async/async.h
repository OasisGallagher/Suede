#pragma once
#include <ZThread/Runnable.h>

class AsyncEventReceiver {
public:
	virtual void OnAsyncFinished(ZThread::Runnable* runnable) = 0;
};

class AsyncWorker : public ZThread::Runnable {
public:
	AsyncWorker(AsyncEventReceiver* receiver) : receiver_(receiver) {}

public:
	// override OnRun instead.
	virtual void run();

protected:
	virtual void OnRun() = 0;

private:
	AsyncEventReceiver* receiver_;
};
