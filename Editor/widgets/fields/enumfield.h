#pragma once
#include <QComboBox>

class EnumField : public QComboBox {
public:
	EnumField(QWidget* parent) : QComboBox(parent) {}

public:
	template <class T>
	void setEnums(T selected);
};

template <class T>
void EnumField::setEnums(T selected) {
	clear();

	int index = -1;
	for (int i = 0; i < T::size(); ++i) {
		if (selected == T::value(i)) { index = i; }
		addItem(T::value(i).to_string());
	}

	setCurrentIndex(index);
}
