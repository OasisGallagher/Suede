#pragma once
#include <QTimer>
#include <QCheckBox>
#include <QStatusBar>

class StatsWidget;

class Status : public QStatusBar {
	Q_OBJECT

public:
	static Status* get();

public:
	Status(QWidget* parent);
	~Status();

protected:
	virtual bool eventFilter(QObject* watched, QEvent* event);

private slots:
	void updateStat();
	void toggleStatistics(int state);

private:
	void moveWidgets();

private:
	QTimer* timer_;

	StatsWidget* stat_;
	QCheckBox* statChk_;
};
