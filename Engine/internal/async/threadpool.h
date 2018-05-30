#pragma once
#include <queue>

#define USE_POOL_EXECUTOR

#include <ZThread/Mutex.h>
#include <ZThread/LockedQueue.h>

#ifdef USE_POOL_EXECUTOR
#include <ZThread/PoolExecutor.h>
#else
#include <ZThread/ThreadedExecutor.h>
#endif

#include "frameeventlistener.h"

class ThreadPool : public FrameEventListener {
public:
	ThreadPool();
	~ThreadPool();

public:
	void OnFinished(ZThread::Runnable* runnable);

protected:
	virtual void OnFrameEnter();
	virtual void OnSchedule(ZThread::Task& schedule) = 0;

protected:
	bool Execute(ZThread::Task task);

private:
	void UpdateSchedules();

private:
#ifdef USE_POOL_EXECUTOR
	ZThread::PoolExecutor executor_;
#else
	ZThread::ThreadedExecutor executor_;
#endif

	std::vector<ZThread::Task> tasks_;
	std::queue<ZThread::Task> schedules_;
	ZThread::Mutex scheduleContainerMutex_;
};
