#pragma once
#include <queue>

#include <ZThread/Mutex.h>
#include <ZThread/Executor.h>
#include <ZThread/LockedQueue.h>

#include "frameeventlistener.h"

class Worker;
class WorkerEventListener {
public:
	virtual void OnWorkFinished(Worker* runnable) = 0;
};

class Worker : public ZThread::Runnable {
public:
	Worker(WorkerEventListener* listener) : listener_(listener) {}

public:
	// override Run instead.
	virtual void run();

protected:
	virtual void Run() = 0;

private:
	WorkerEventListener* listener_;
};

class ThreadPool : public FrameEventListener, public WorkerEventListener {
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

public:
	virtual void OnWorkFinished(Worker* runnable);

protected:
	virtual void OnFrameEnter();
	virtual void OnSchedule(ZThread::Task& schedule) = 0;

protected:
	bool Execute(ZThread::Task task);

private:
	void UpdateSchedules();
	void CreateExecutor(int type);

private:
	ZThread::Executor* executor_;

	std::vector<ZThread::Task> tasks_;
	std::queue<ZThread::Task> schedules_;
	ZThread::Mutex scheduleContainerMutex_;
};
