#include "async.h"
#include "engine.h"
#include "threadpool.h"
#include "debug/debug.h"
#include "memory/refptr.h"
#include "frameeventqueue.h"

#include <ZThread/PoolExecutor.h>
#include <ZThread/ThreadedExecutor.h>
#include <ZThread/ConcurrentExecutor.h>
#include <ZThread/SynchronousExecutor.h>

void Worker::run() {
	Run();
	workFinished.fire(this);
}

ThreadPool::ThreadPool(int type) {
	Engine::frameEnter.subscribe(this, &ThreadPool::OnFrameEnter, (int)FrameEventQueue::User);
	CreateExecutor(type);
}

ThreadPool::~ThreadPool() {
	Engine::frameEnter.unsubscribe(this);

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

	delete executor_;
}

void ThreadPool::CreateExecutor(int type) {
	if (type == Threaded) {
		executor_ = new ZThread::ThreadedExecutor;
	}
	else if (type == Concurrent) {
		executor_ = new ZThread::ConcurrentExecutor;
	}
	else if (type == Synchronous) {
		executor_ = new ZThread::SynchronousExecutor;
	}
	else {
		if (type <= 0) {
			Debug::LogError("invalid parameter for thread pool");
		}

		executor_ = new ZThread::PoolExecutor(type);
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

bool ThreadPool::Execute(Worker* worker) {
	worker->workFinished.subscribe(this, &ThreadPool::OnWorkFinished);

	ZThread::Task task(worker);
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
