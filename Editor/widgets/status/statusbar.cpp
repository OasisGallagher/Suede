#include <QLabel>

#include "time2.h"
#include "statusbar.h"
#include "debug/debug.h"

StatusBar::StatusBar(QWidget* parent) :QStatusBar(parent) {
	QLabel* label = new QLabel(this);
	addWidget(label);
	label->setText("Ready");
}

StatusBar::~StatusBar() {
}
