#include "globjectmaintainer.h"

#include "context.h"

GLObjectMaintainer::GLObjectMaintainer(Context* context) : context_(context) {
	context_->destroyed.subscribe(this, &GLObjectMaintainer::OnContextDestroyed);
}

GLObjectMaintainer::~GLObjectMaintainer() {
	if (context_ != nullptr) {
		context_->destroyed.unsubscribe(this);
	}
}
