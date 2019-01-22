#pragma once
#include <QTimer>
#include <QCheckBox>
#include <QStatusBar>

class StatusBar : public QStatusBar {
	Q_OBJECT

public:
	static StatusBar* instance();

public:
	StatusBar(QWidget* parent);
	~StatusBar();
};
