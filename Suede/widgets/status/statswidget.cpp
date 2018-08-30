#include "statswidget.h"

#include "statistics.h"

StatsWidget::StatsWidget(QWidget* parent) : QWidget(parent) {
	ui.setupUi(this);
	setStyleSheet("#" + objectName() + "{ border: 1px solid gray }");
}

void StatsWidget::updateContent() {
	ui.fps->setText(QString::number(Statistics::instance()->GetFrameRate(), 'f', 2));

	ui.culling->setText(QString::asprintf("%.2f ms", Statistics::instance()->GetCullingElapsed() * 1000));
	ui.rendering->setText(QString::asprintf("%.2f ms", Statistics::instance()->GetRenderingElapsed() * 1000));

	ui.drawcalls->setText(QString::number(Statistics::instance()->GetDrawcalls()));
	ui.triangles->setText(QString::number(Statistics::instance()->GetTriangles()));
}
