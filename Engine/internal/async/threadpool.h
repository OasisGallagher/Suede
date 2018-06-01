#pragma once
#include <queue>

#include <ZThread/Mutex.h>
#include <ZThread/LockedQueue.h>

#include <ZThread/Executor.h>

#include "async.h"
#include "frameeventlistener.h"

class ThreadPool : public FrameEventListener, public AsyncEventListener {
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
	virtual void OnAsyncFinished(ZThread::Runnable* runnable);

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
