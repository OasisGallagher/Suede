#include <QThread>
#include <QTimerEvent>

#include "time2.h"
#include "status.h"
#include "statistics.h"
#include "debug/debug.h"

#define FPS_UPDATE_INTERVAL		800

Status* statusInstance;
Status* Status::get() {
	return statusInstance;
}

Status::Status(QWidget* parent) :QStatusBar(parent) {
	statusInstance = this;
	QLabel* label = new QLabel(this);
	addWidget(label);
	label->setText(tr("Ready"));

	fps_ = new QLabel(this);
	addPermanentWidget(fps_);
	fps_->setText("0.00");

	timer_ = startTimer(FPS_UPDATE_INTERVAL);
}

Status::~Status() {
	killTimer(timer_);
}

void Status::timerEvent(QTimerEvent *event) {
	if (event->timerId() == timer_) {
		fps_->setText(
			QString("triangles (%1), drawcalls (%2), %3 fps")
			.arg(Statistics::GetTriangles())
			.arg(Statistics::GetDrawcalls())
			.arg(QString::number(Statistics::GetFrameRate(), 'f', 2)));
	}
}
