#include <QLabel>

#include "time2.h"
#include "statusbar.h"
#include "statistics.h"
#include "debug/debug.h"

StatusBar* statusBarInstance;
StatusBar* StatusBar::instance() {
	return statusBarInstance;
}

StatusBar::StatusBar(QWidget* parent) :QStatusBar(parent) {
	statusBarInstance = this;

	QLabel* label = new QLabel(this);
	addWidget(label);
	label->setText("Ready");
}

StatusBar::~StatusBar() {
}
