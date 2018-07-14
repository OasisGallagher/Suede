#include "vecfield.h"

#include <QLabel>
#include <QHBoxLayout>
#include "floatfield.h"

#define MAX_FLOAT_FIELDS	4

VecField::VecField(QWidget* parent, const QStringList& names) : QWidget(parent) {
	count_ = names.size();
	Q_ASSERT(count_ <= MAX_FLOAT_FIELDS);

	QHBoxLayout* layout = new QHBoxLayout(this);
	layout->setContentsMargins(0, 0, 0, 0);

	fields_ = new FloatField*[count_];
	for (uint i = 0; i < count_; ++i) {
		QLabel* label = new QLabel(names[i], this);
		fields_[i] = new FloatField(this);

		layout->addWidget(label);
		layout->addWidget(fields_[i]);
		connect(fields_[i], SIGNAL(valueChanged(float)), this, SLOT(onFieldValueChanged(float)));
	}
}

VecField::~VecField() {
	delete[] fields_;
}

void VecField::setFields(float* values) {
	for (uint i = 0; i < count_; ++i) {
		fields_[i]->setValue(values[i]);
	}
}

void VecField::onFieldValueChanged(float value) {
	static float values[MAX_FLOAT_FIELDS];
	for (uint i = 0; i < count_; ++i) {
		values[i] = fields_[i]->value();
	}

	valueChanged(values);
}

const QStringList& VecField::verifyCount(const QStringList& names, uint count) {
	Q_ASSERT(names.size() == count);
	return names;
}
