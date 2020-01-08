#include "threadpool.h"

#include "engine.h"
#include "frameevents.h"
#include "debug/debug.h"

ThreadPool::ThreadPool(uint n) : busy_(0U), stopped_(false) {
	workers_.resize(n, std::bind(&ThreadPool::ThreadProc, this));
}

ThreadPool::~ThreadPool() {
	// set stop-condition
	std::unique_lock<std::mutex> lock(tasks_mutex_);
	stopped_ = true;
	cv_task_.notify_all();
	lock.unlock();

	// all threads terminate, then we're done.
	for (auto& worker : workers_) {
		worker.join();
	}
}

void ThreadPool::ThreadProc() {
	while (true) {
		std::unique_lock<std::mutex> lock(this->tasks_mutex_);
		cv_task_.wait(lock, [this]() { return this->stopped_ || !this->tasks_.empty(); });

		if (this->stopped_ && this->tasks_.empty()) {
			return;
		}

		// got work. set busy.
		++busy_;

		// pull from queue
		auto task = tasks_.front();
		tasks_.pop_front();

		// release lock. run async
		lock.unlock();

		// run function outside context
		task->Run();

		OnTaskFinished(task.get());

		lock.lock();
		--busy_;
		cv_finished_.notify_one();
	}
}

void ThreadPool::AddTask(Task* task) {
	std::unique_lock<std::mutex> lock(tasks_mutex_);
	tasks_.emplace_back(task);
	cv_task_.notify_one();
}

// waits until the queue is empty.
void ThreadPool::WaitFinished() {
	std::unique_lock<std::mutex> lock(tasks_mutex_);
	cv_finished_.wait(lock, [this]() { return tasks_.empty() && (busy_ == 0); });
}

ScheduledThreadPool::ScheduledThreadPool(uint n) : ThreadPool(n) {
	Engine::GetSubsystem<FrameEvents>()->frameEnter.subscribe(this, &ScheduledThreadPool::OnFrameEnter, (int)FrameEventQueue::User);
}

ScheduledThreadPool::~ScheduledThreadPool() {
	Engine::GetSubsystem<FrameEvents>()->frameEnter.unsubscribe(this);
}

void ScheduledThreadPool::OnTaskFinished(Task* task) {
	ThreadPool::OnTaskFinished(task);

	std::lock_guard<std::mutex> _lock(schedules_mutex);
	schedules.push(task);
}

void ScheduledThreadPool::OnFrameEnter() {
	std::lock_guard<std::mutex> lock(schedules_mutex);

	for (; !schedules.empty();) {
		ref_ptr<Task> task = schedules.front();
		schedules.pop();

		OnSchedule(task.get());
	}
}
