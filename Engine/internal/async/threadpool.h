#pragma once

#include <queue>
#include <deque>
#include <vector>
#include <thread>
#include <functional>

#include <mutex>
//#include <atomic>
#include <condition_variable>

#include "memory/refptr.h"
#include "containers/dynamicarray.h"

// https://stackoverflow.com/a/23899225/2705388
class Task : public intrusive_ref_counter {
public:
	virtual void Run() = 0;
};

class ThreadPool {
public:
	ThreadPool(unsigned n);
	~ThreadPool();

	void WaitFinished();
	void AddTask(Task* task);

protected:
	virtual void OnSchedule(Task* task) {}

private:
	void ThreadProc();
	void OnFrameEnter();

private:
	dynamic_array<std::thread> workers;

	std::mutex tasks_mutex;
	std::deque<ref_ptr<Task>> tasks;

	std::mutex schedules_mutex;
	std::queue<ref_ptr<Task>> schedules;

	std::condition_variable cv_task;
	std::condition_variable cv_finished;

	bool stopped;
	unsigned busy;
};
