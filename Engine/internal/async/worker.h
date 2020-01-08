#pragma once
#include <mutex>
#include <thread>

class Worker {
public:
	Worker();
	~Worker();


public:
	virtual void Stop();
	std::thread::id GetThreadID() const { return thread_.get_id(); }

protected:
	virtual bool OnWork() = 0;

protected:
	void SetWorking(bool value);

	bool IsStopped() const { return stopped_; }
	bool IsWorking() const { return working_; }

private:
	bool stopped_ = false;
	bool working_ = false; 
	
	std::mutex mutex_;
	std::thread thread_;
	std::condition_variable cond_;
};
