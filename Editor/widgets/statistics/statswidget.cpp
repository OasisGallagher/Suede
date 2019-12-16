#include "statswidget.h"

#include "statistics.h"

StatsWidget::StatsWidget(QWidget* parent) : QWidget(parent) {
	ui.setupUi(this);
	//setWindowFlags(Qt::FramelessWindowHint);
	setStyleSheet("#" + objectName() + "{ border: 1px solid black }");

	timer_ = new QTimer(this);
	connect(timer_, &QTimer::timeout, this, &StatsWidget::updateContent);
	timer_->start(800);
}

void StatsWidget::updateContent() {
	if (isVisible()) {
		ui.fps->setText(QString::number(Statistics::GetFrameRate(), 'f', 2));

		ui.script->setText(QString::asprintf("%.2f ms", Statistics::GetScriptElapsed() * 1000));
		ui.culling->setText(QString::asprintf("%.2f ms", Statistics::GetCullingElapsed() * 1000));
		ui.rendering->setText(QString::asprintf("%.2f ms", Statistics::GetRenderingElapsed() * 1000));

		ui.drawcalls->setText(QString::number(Statistics::GetDrawcalls()));
		ui.triangles->setText(QString::number(Statistics::GetTriangles()));
	}
}

void StatsWidget::showEvent(QShowEvent *event) {
	updateContent();
	QWidget::showEvent(event);
	raise();
}
