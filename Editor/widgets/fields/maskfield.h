#pragma once

#include <QComboBox>
#include <QCheckBox>
#include <QListWidget>

class MaskField : public QComboBox {
	Q_OBJECT

public:
	MaskField(QWidget* parent);

public:
	template <class T>
	void setEnums(T selected);
	void setItems(const QStringList& items, int mask);

signals:
	void selectionChanged(int mask);

private slots:
	void onSelectedChanged(int state);

private:
	void updateCheckBoxes();

	void setSelectedMask(int value);
	void addSelectedMask(int index);
	void removeSelectedMask(int index);

	int addItem(const QString& str);
	void updateEverythingMask(int size);
	
	void updateText();
	uint count1Bits(uint x);

	void clearAll();
	enum {
		MaxMaskItems = sizeof(int) * 8,
	};

private:

	int selected_;
	int everything_;
	QListWidget* view_;
	QList<int> values_;
	QList<QCheckBox*> checkBoxes_;
};

template <class T>
void MaskField::setEnums(T selected) {
	QStringList list;
	values_.clear();

	// skip None & Everything.
	for (int i = 2; i < T::size(); ++i) {
		list << T::value(i).to_string();
		values_.push_back(T::value(i));
	}

	setItems(list, selected);
}
