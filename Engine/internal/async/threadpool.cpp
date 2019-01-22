#include "async.h"
#include "engine.h"
#include "threadpool.h"
#include "debug/debug.h"
#include "memory/memory.h"

#include <ZThread/PoolExecutor.h>
#include <ZThread/ThreadedExecutor.h>
#include <ZThread/ConcurrentExecutor.h>
#include <ZThread/SynchronousExecutor.h>

void Worker::run() {
	Run();

	if (listener_ != nullptr) {
		listener_->OnWorkFinished(this);
	}
}

ThreadPool::ThreadPool(int type) {
	Engine::AddFrameEventListener(this);
	CreateExecutor(type);
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
	catch (const std::exception& e) {
		Debug::LogError(e.what());
	}

	MEMORY_DELETE(executor_);
}

void ThreadPool::CreateExecutor(int type) {
	if (type == Threaded) {
		executor_ = MEMORY_NEW(ZThread::ThreadedExecutor);
	}
	else if (type == Concurrent) {
		executor_ = MEMORY_NEW(ZThread::ConcurrentExecutor);
	}
	else if (type == Synchronous) {
		executor_ = MEMORY_NEW(ZThread::SynchronousExecutor);
	}
	else {
		if (type <= 0) {
			Debug::LogError("invalid parameter for thread pool");
		}

		executor_ = MEMORY_NEW(ZThread::PoolExecutor, type);
	}
}

void ThreadPool::OnWorkFinished(Worker* runnable) {
	ZTHREAD_LOCK_SCOPE(scheduleContainerMutex_);

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
	catch (const std::exception& e) {
		Debug::LogError(e.what());
	}
}

void ThreadPool::UpdateSchedules() {
	ZTHREAD_LOCK_SCOPE(scheduleContainerMutex_);

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
	catch (const std::exception& e) {
		Debug::LogError(e.what());
		return false;
	}

	return true;
}
