#include "engine.h"
#include "threadpool.h"
#include "debug/debug.h"
#include "memory/memory.h"

#include <ZThread/PoolExecutor.h>
#include <ZThread/ThreadedExecutor.h>
#include <ZThread/ConcurrentExecutor.h>
#include <ZThread/SynchronousExecutor.h>

#define LogUnknownException()	Debug::LogError("unknown exception")

ThreadPool::ThreadPool(int type) {
	Engine::AddFrameEventListener(this);
	CreateExecutor(type);
}

void ThreadPool::CreateExecutor(int type) {
	if (type == Threaded) {
		executor_ = MEMORY_CREATE(ZThread::ThreadedExecutor);
	}
	else if (type == Concurrent) {
		executor_ = MEMORY_CREATE(ZThread::ConcurrentExecutor);
	}
	else if (type == Synchronous) {
		executor_ = MEMORY_CREATE(ZThread::SynchronousExecutor);
	}
	else {
		if (type <= 0) {
			Debug::LogError("invalid parameter for thread pool");
		}

		executor_ = MEMORY_CREATE(ZThread::PoolExecutor, type);
	}
}

ThreadPool::~ThreadPool() {
	Engine::RemoveFrameEventListener(this);

	try {
		executor_->interrupt();
		executor_->wait();
	}
	catch (const ZThread::Synchronization_Exception& e) {
		Debug::LogError(e.what());
	}
	catch (...) {
		LogUnknownException();
	}

	MEMORY_RELEASE(executor_);
}

void ThreadPool::OnAsyncFinished(ZThread::Runnable* runnable) {
	ZThread::Guard<ZThread::Mutex> guard(scheduleContainerMutex_);

	for (std::vector<ZThread::Task>::iterator ite = tasks_.begin(); ite != tasks_.end(); ++ite) {
		ZThread::Task task = *ite;
		if (runnable == task.get()) {
			tasks_.erase(ite);
			schedules_.push(task);
			break;
		}
	}
}

void ThreadPool::OnFrameEnter() {
	try {
		UpdateSchedules();
	}
	catch (const ZThread::Synchronization_Exception& e) {
		Debug::LogError(e.what());
	}
	catch (...) {
		LogUnknownException();
	}
}

void ThreadPool::UpdateSchedules() {
	ZThread::Guard<ZThread::Mutex> guard(scheduleContainerMutex_);

	for (; !schedules_.empty();) {
		ZThread::Task schedule = schedules_.front();
		schedules_.pop();

		OnSchedule(schedule);
	}
}

bool ThreadPool::Execute(ZThread::Task task) {
	tasks_.push_back(task);

	try {
		executor_->execute(task);
	}
	catch (const ZThread::Synchronization_Exception& e) {
		Debug::LogError(e.what());
		return false;
	}
	catch (...) {
		LogUnknownException();
		return false;
	}

	return true;
}
