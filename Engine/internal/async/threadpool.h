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
	friend class ThreadPool;
	virtual void Run() = 0;
};

class ThreadPool {
public:
	ThreadPool(uint n);
	~ThreadPool();

	void WaitFinished();
	void AddTask(Task* task);

protected:
	virtual void OnTaskFinished(Task* task) {}

private:
	void ThreadProc();

private:
	dynamic_array<std::thread> workers_;

	std::mutex tasks_mutex_;
	std::deque<ref_ptr<Task>> tasks_;

	std::condition_variable cv_task_;
	std::condition_variable cv_finished_;

	uint busy_;
	bool stopped_;
};

class ScheduledThreadPool : public ThreadPool {
public:
	ScheduledThreadPool(uint n);
	~ScheduledThreadPool();

protected:
	virtual void OnSchedule(Task* task) = 0;
	virtual void OnTaskFinished(Task* task) final;

private:
	void OnFrameEnter();

private:
	std::mutex schedules_mutex;
	std::queue<ref_ptr<Task>> schedules;
};
