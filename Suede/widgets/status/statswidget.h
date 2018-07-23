#pragma once

#include <QWidget>

#include "ui_statswidget.h"

class StatsWidget : public QWidget {
public:
	StatsWidget(QWidget* parent);
	~StatsWidget() {}

public:
	void setStats(float fps, uint drawcalls, uint triangles);

private:
	Ui::StatsWidget ui;
};