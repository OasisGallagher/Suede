#include <QLabel>

#include "time2.h"
#include "status.h"
#include "statistics.h"
#include "debug/debug.h"

Status* statusInstance;
Status* Status::instance() {
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
