#include "statswidget.h"

#include "engine.h"
#include "profiler.h"

StatsWidget::StatsWidget(QWidget* parent) : QWidget(parent) {
	ui.setupUi(this);
	setAttribute(Qt::WA_StyledBackground, true);
	setStyleSheet("#" + objectName() + "{ border: 1px solid gray }");

	fps_ = addRecord("FPS:");
	script_ = addRecord("Script:");
	culling_ = addRecord("Culling:");
	cullingUpdate_ = addRecord("Culling Update:");
	visibleGameObject_ = addRecord("Visible:");
	rendering_ = addRecord("Rendering:");
	drawcalls_ = addRecord("Drawcalls:");
	triangles_ = addRecord("Traingles:");

	adjustSize();

	timer_ = new QTimer(this);
	connect(timer_, &QTimer::timeout, this, &StatsWidget::updateContent);
	timer_->start(800);
}

void StatsWidget::updateContent() {
	if (isVisible()) {
		const StatisticInfo* stats = Engine::GetSubsystem<Profiler>()->GetStatisticInfo();
		fps_->setText(QString::number(stats->frameRate, 'f', 2));

		script_->setText(QString::asprintf("%.2f ms", stats->scriptElapsed * 1000));
		culling_->setText(QString::asprintf("%.2f ms", stats->cullingElapsed * 1000));
		cullingUpdate_->setText(QString::asprintf("%.2f ms", stats->cullingUpdateElapsed * 1000));
		visibleGameObject_->setText(QString::asprintf("%u/%u", stats->visibleGameObject, stats->totalGameObject));

		rendering_->setText(QString::asprintf("%.2f ms", stats->renderingElapsed * 1000));

		drawcalls_->setText(QString::number(stats->ndrawcalls));
		triangles_->setText(QString::number(stats->ntriangles));

		adjustSize();
	}
}

void StatsWidget::showEvent(QShowEvent *event) {
	updateContent();
	QWidget::showEvent(event);
	raise();
}

QLabel* StatsWidget::addRecord(const QString& label) {
	QLabel* text = new QLabel(this);
	ui.formLayout->addRow(label, text);
	return text;
}
