#include "statswidget.h"

StatsWidget::StatsWidget(QWidget* parent) : QWidget(parent) {
	ui.setupUi(this);
	setStyleSheet("#" + objectName() + "{ border: 1px solid gray }");
}

void StatsWidget::setStats(float fps, uint drawcalls, uint triangles) {
	ui.fps->setText(QString::number(fps, 'f', 2));
	ui.drawcalls->setText(QString::number(drawcalls));
	ui.triangles->setText(QString::number(triangles));
}
