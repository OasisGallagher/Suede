#include "statwidget.h"

StatWidget::StatWidget(QWidget* parent) : QWidget(parent) {
	ui.setupUi(this);
	setStyleSheet("#" + objectName() + "{ border: 1px solid gray }");
}

void StatWidget::setFps(float value) {
	ui.fps->setText(QString::number(value, 'f', 2));
}

void StatWidget::setDrawcalls(uint value) {
	ui.drawcalls->setText(QString::number(value));
}

void StatWidget::setTriangles(uint value) {
	ui.triangles->setText(QString::number(value));
}
