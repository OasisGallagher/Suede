#include "worker.h"
#include "debug/debug.h"

Worker::Worker() {
	thread_ = std::thread([this]() {
		for (;;) {
			std::unique_lock<std::mutex> lock(mutex_);
			cond_.wait(lock, [this]() {
				return this->stopped_ || this->working_;
			});

			if (stopped_) { break; }
			SUEDE_ASSERT(working_);

			bool finished = OnWork();
			SetWorking(!finished);
		}
	});
}

Worker::~Worker() {
	Stop();
}

void Worker::Stop() {
	if (!stopped_) {
		stopped_ = true;
		cond_.notify_all();

		thread_.join();
	}
}

void Worker::SetWorking(bool value) {
	if (working_ != value) {
		working_ = value;
		if (working_) { cond_.notify_one(); }
	}
}
