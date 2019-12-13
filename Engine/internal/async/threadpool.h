#pragma once
#include <queue>

#include <ZThread/Mutex.h>
#include <ZThread/Executor.h>
#include <ZThread/LockedQueue.h>

#include "tools/event.h"

class Worker;

class Worker : public ZThread::Runnable {
public:
	Worker() {}

public:
	// override Run instead.
	virtual void run();

public:
	event<Worker*> workFinished;

protected:
	virtual void Run() = 0;
};

class ThreadPool {
public:
	enum {
		Threaded = -2,
		Concurrent = -1,
		Synchronous = 0,
		// Pool >= 1
	};

public:
	ThreadPool(int type);
	~ThreadPool();

protected:
	virtual void OnSchedule(ZThread::Task& schedule) = 0;

protected:
	bool Execute(Worker* woker);

private:
	void OnFrameEnter();

	void UpdateSchedules();
	void CreateExecutor(int type);
	void OnWorkFinished(Worker* runnable);

private:
	ZThread::Executor* executor_;

	std::vector<ZThread::Task> tasks_;
	std::queue<ZThread::Task> schedules_;
	ZThread::Mutex scheduleContainerMutex_;
};
