#pragma once
#include <QTimer>
#include <QCheckBox>
#include <QStatusBar>

class StatusBar : public QStatusBar {
	Q_OBJECT

public:
	StatusBar(QWidget* parent);
	~StatusBar();
};
