#include "inspector.h"

static Inspector* inspectorInstance;

Inspector* Inspector::get() {
	return inspectorInstance;
}

Inspector::Inspector(QWidget* parent) : QDockWidget(parent) {
	inspectorInstance = this;
}

Inspector::~Inspector() {
	inspectorInstance = nullptr;
}

void Inspector::init() {
}
