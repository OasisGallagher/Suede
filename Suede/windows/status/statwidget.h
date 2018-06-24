#pragma once

#include <QWidget>

#include "ui_statwidget.h"

class StatWidget : public QWidget {
public:
	StatWidget(QWidget* parent);
	~StatWidget() {}

public:
	void setFps(float value);
	void setDrawcalls(uint value);
	void setTriangles(uint value);

private:
	Ui::StatWidget ui;
};