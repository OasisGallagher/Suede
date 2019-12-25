#include "threadpool.h"

#include "world.h"
#include "debug/debug.h"
#include "frameeventqueue.h"

ThreadPool::ThreadPool(unsigned n) : busy(0U), stopped(false) {
	workers.resize(n, std::bind(&ThreadPool::ThreadProc, this));
	World::frameEnter().subscribe(this, &ThreadPool::OnFrameEnter, (int)FrameEventQueue::User);
}

ThreadPool::~ThreadPool() {
	World::frameEnter().unsubscribe(this);

	// set stop-condition
	std::unique_lock<std::mutex> lock(tasks_mutex);
	stopped = true;
	cv_task.notify_all();
	lock.unlock();

	// all threads terminate, then we're done.
	for (auto& worker : workers) {
		worker.join();
	}
}

void ThreadPool::ThreadProc() {
	while (true) {
		std::unique_lock<std::mutex> lock(this->tasks_mutex);
		cv_task.wait(lock, [this]() { return this->stopped || !this->tasks.empty(); });

		if (this->stopped && this->tasks.empty()) {
			return;
		}

		// got work. set busy.
		++busy;

		// pull from queue
		auto task = tasks.front();
		tasks.pop_front();

		// release lock. run async
		lock.unlock();

		// run function outside context
		task->Run();

		{
			std::lock_guard<std::mutex> _lock(schedules_mutex);
			schedules.push(task);
		}

		lock.lock();
		--busy;
		cv_finished.notify_one();
	}
}

void ThreadPool::OnFrameEnter() {
	std::lock_guard<std::mutex> lock(schedules_mutex);

	for (; !schedules.empty();) {
		ref_ptr<Task> task = schedules.front();
		schedules.pop();

		OnSchedule(task.get());
	}
}

void ThreadPool::AddTask(Task* task) {
	std::unique_lock<std::mutex> lock(tasks_mutex);
	tasks.emplace_back(task);
	cv_task.notify_one();
}

// waits until the queue is empty.
void ThreadPool::WaitFinished() {
	std::unique_lock<std::mutex> lock(tasks_mutex);
	cv_finished.wait(lock, [this]() { return tasks.empty() && (busy == 0); });
}
