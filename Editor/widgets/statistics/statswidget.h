#pragma once

#include <QWidget>

#include "ui_statswidget.h"

class StatsWidget : public QWidget {
public:
	StatsWidget(QWidget* parent);
	~StatsWidget() {}

public:
	void updateContent();

private:
	Ui::StatsWidget ui;
};
