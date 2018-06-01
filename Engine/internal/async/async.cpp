#include "async.h"

void AsyncWorker::run() {
	OnRun();

	if (listener_ != nullptr) {
		listener_->OnAsyncFinished(this);
	}
}
