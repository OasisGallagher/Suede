#include "statswidget.h"

#include "statistics.h"

StatsWidget::StatsWidget(QWidget* parent) : QWidget(parent) {
	ui.setupUi(this);
	setStyleSheet("#" + objectName() + "{ border: 1px solid gray }");
}

void StatsWidget::updateContent() {
	ui.fps->setText(QString::number(Statistics::GetFrameRate(), 'f', 2));

	ui.script->setText(QString::asprintf("%.2f ms", Statistics::GetScripeElapsed() * 1000));
	ui.culling->setText(QString::asprintf("%.2f ms", Statistics::GetCullingElapsed() * 1000));
	ui.rendering->setText(QString::asprintf("%.2f ms", Statistics::GetRenderingElapsed() * 1000));

	ui.drawcalls->setText(QString::number(Statistics::GetDrawcalls()));
	ui.triangles->setText(QString::number(Statistics::GetTriangles()));
}
