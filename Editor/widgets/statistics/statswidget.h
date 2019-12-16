#pragma once

#include <QTimer>
#include <QWidget>

#include "ui_statswidget.h"

class StatsWidget : public QWidget {
	Q_OBJECT

public:
	StatsWidget(QWidget* parent);
	~StatsWidget() {}

public:
	void updateContent();

protected:
	virtual void showEvent(QShowEvent *event);

private:
	QTimer* timer_;
	Ui::StatsWidget ui;
};
