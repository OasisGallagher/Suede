#include "async.h"

void AsyncWorker::run() {
	OnRun();

	if (receiver_ != nullptr) {
		receiver_->OnAsyncFinished(this);
	}
}

