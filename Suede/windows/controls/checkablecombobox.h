#pragma once
#include <QComboBox>
#include <QCheckBox>
#include <QListWidget>

class CheckableComboBox : public QComboBox {
	Q_OBJECT

public:
	CheckableComboBox(QWidget* parent = Q_NULLPTR);

signals:
	void selectionChanged(uint mask);

public:
	void setItems(const QStringList& items);

private slots:
	void onSelectedChanged(int state);

private:
	void clearAll();
	void updateText();
	int minimumWidth();
	uint count1Bits(uint x, int& p);

private:
	uint selected_;

	QListWidget* view_;
	QList<QCheckBox*> checkBoxes_;
};