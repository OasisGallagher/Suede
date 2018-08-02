#pragma once
#include <QTimer>
#include <QCheckBox>
#include <QStatusBar>

class Status : public QStatusBar {
	Q_OBJECT

public:
	static Status* instance();

public:
	Status(QWidget* parent);
	~Status();
};
