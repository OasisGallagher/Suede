#include "time2.h"
#include "status.h"
#include "statswidget.h"
#include "statistics.h"
#include "debug/debug.h"

#define FPS_UPDATE_INTERVAL		800

Status* statusInstance;
Status* Status::get() {
	return statusInstance;
}

Status::Status(QWidget* parent) :QStatusBar(parent), stat_(nullptr) {
	statusInstance = this;

	QLabel* label = new QLabel(this);
	addWidget(label);
	label->setText(tr("Ready"));

	statChk_ = new QCheckBox(this);
	connect(statChk_, SIGNAL(stateChanged(int)), this, SLOT(toggleStatistics(int)));
	statChk_->setText("Stat");
	addPermanentWidget(statChk_);

	statChk_->installEventFilter(this);

	timer_ = new QTimer(this);
	connect(timer_, SIGNAL(timeout()), this, SLOT(updateStat()));
	timer_->start(FPS_UPDATE_INTERVAL);
}

Status::~Status() {
}

bool Status::eventFilter(QObject* watched, QEvent* event) {
	if (event->type() == QEvent::Move && watched == statChk_) {
		moveWidgets();
	}

	return false;
}

void Status::toggleStatistics(int state) {
	if (stat_ == nullptr) {
		stat_ = new StatsWidget(parentWidget());
		moveWidgets();
	}

	stat_->setVisible(!!state);
	if (stat_->isVisible()) {
		updateStat();
		moveWidgets();
	}
}

void Status::updateStat() {
	if (stat_ != nullptr && stat_->isVisible()) {
		stat_->setStats(Statistics::GetFrameRate(),
			Statistics::GetDrawcalls(),
			Statistics::GetTriangles()
		);
	}
}

void Status::moveWidgets() {
	if (stat_ != nullptr) {
		QPoint pos = mapToParent(statChk_->pos());
		pos.setX(pos.x() - stat_->width());
		pos.setY(pos.y() - stat_->height());
		stat_->move(pos);
	}
}
