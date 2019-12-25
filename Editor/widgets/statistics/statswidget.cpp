#include "statswidget.h"

#include "world.h"

StatsWidget::StatsWidget(QWidget* parent) : QWidget(parent) {
	ui.setupUi(this);
	setAttribute(Qt::WA_StyledBackground, true);
	setStyleSheet("#" + objectName() + "{ border: 1px solid gray }");

	timer_ = new QTimer(this);
	connect(timer_, &QTimer::timeout, this, &StatsWidget::updateContent);
	timer_->start(800);
}

void StatsWidget::updateContent() {
	if (isVisible()) {
		const FrameStatistics* stats = World::GetFrameStatistics();
		ui.fps->setText(QString::number(stats->frameRate, 'f', 2));

		ui.script->setText(QString::asprintf("%.2f ms", stats->scriptElapsed * 1000));
		ui.culling->setText(QString::asprintf("%.2f ms", stats->cullingElapsed * 1000));
		ui.rendering->setText(QString::asprintf("%.2f ms", stats->renderingElapsed * 1000));

		ui.drawcalls->setText(QString::number(stats->ndrawcalls));
		ui.triangles->setText(QString::number(stats->ntriangles));
	}
}

void StatsWidget::showEvent(QShowEvent *event) {
	updateContent();
	QWidget::showEvent(event);
	raise();
}
