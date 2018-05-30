#include "engine.h"
#include "threadpool.h"
#include "debug/debug.h"

ThreadPool::ThreadPool()
#ifdef USE_POOL_EXECUTOR
	: executor_(16)
#endif
{
	Engine::AddFrameEventListener(this);
}

ThreadPool::~ThreadPool() {
	Engine::RemoveFrameEventListener(this);

	try {
		executor_.interrupt();
		executor_.wait();
	}
	catch (const ZThread::Synchronization_Exception& e) {
		Debug::LogError(e.what());
	}
	catch (...) {
		Debug::LogError("unknown exception");
	}
}

void ThreadPool::OnFinished(ZThread::Runnable* runnable) {
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
		Debug::LogError("unknown exception");
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
		executor_.execute(task);
	}
	catch (const ZThread::Synchronization_Exception& e) {
		Debug::LogError(e.what());
		return false;
	}

	return true;
}
