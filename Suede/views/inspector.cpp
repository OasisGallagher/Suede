#include "inspector.h"

Inspector* Inspector::get() {
	static Inspector instance;
	return &instance;
}

void Inspector::initialize() {
	view_->setSize(40, 200);
}
