#include <QThread>
#include <QTimerEvent>

#include "time2.h"
#include "status.h"
#include "debug/debug.h"
#define TIMER_INTERVAL		0.5f

Status* statusInstance;
Status* Status::get() {
	return statusInstance;
}

Status::Status(QWidget* parent) :QStatusBar(parent) {
	statusInstance = this;
	QLabel* label = new QLabel(this);
	addWidget(label);
	label->setText(tr("Ready"));
}

Status::~Status() {
}
