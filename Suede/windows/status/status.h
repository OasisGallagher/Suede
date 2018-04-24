#pragma once
#include <QList>
#include <QLabel>
#include <QStatusBar>

class Status : public QStatusBar {
	Q_OBJECT

public:
	static Status* get();

private:
	friend class Suede;
	Status(QWidget* parent);
	~Status();
};