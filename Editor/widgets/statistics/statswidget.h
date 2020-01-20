#pragma once

#include <QTimer>
#include <QWidget>
#include <QLabel>

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
	QLabel* addRecord(const QString& label);

private:
	QTimer* timer_;
	Ui::StatsWidget ui;

	QLabel* fps_, *script_, *culling_, *cullingUpdate_, *visibleGameObject_, *rendering_, *drawcalls_, *triangles_;
};
