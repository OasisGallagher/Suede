#include <QEvent>
#include <QMouseEvent>
#include <QHBoxLayout>

#include "rangefield.h"
#include "floatfield.h"
#include "tools/math2.h"

RangeField::RangeField(QWidget* parent) : QWidget(parent), min_(0), max_(1) {
	QHBoxLayout* layout = new QHBoxLayout(this);
	layout->setMargin(0);

	edit_ = new FloatField(this, 2);
	edit_->setFixedWidth(40);

	slider_ = new QSlider(Qt::Horizontal, this);
	slider_->installEventFilter(this);

	connect(edit_, SIGNAL(valueChanged(float)), this, SLOT(onEditValue(float)));
	connect(slider_, SIGNAL(valueChanged(int)), this, SLOT(onSlideValue(int)));

	layout->addWidget(slider_);
	layout->addWidget(edit_);
}

void RangeField::setRange(float min, float max) {
	if (min >= max) {
		Debug::LogError("invalid range");
		max = min + 1;
	}

	min_ = min, max_ = max;
	slider_->setRange(0, (max - min) * 100);
}

float RangeField::value() const {
	return min_ + slider_->value() / 100.f;
}

void RangeField::setValue(float value) {
	value = Math::Clamp(value, min_, max_);

	edit_->setValue(value);
	slider_->setValue((value - min_) * 100);

	emit valueChanged(value);
}

void RangeField::onEditValue(float value) {
	slider_->setValue((value - min_) * 100);
	emit valueChanged(value);
}

void RangeField::onSlideValue(int value) {
	float f = min_ + value / 100.f;
	edit_->setValue(f);

	emit valueChanged(f);
}

bool RangeField::eventFilter(QObject* watched, QEvent* event) {
	if (watched == slider_ && event->type() == QEvent::MouseButtonPress) {
		QMouseEvent* mouseEvent = (QMouseEvent*)event;
		if ((mouseEvent->button() & Qt::LeftButton) != 0) {
			float pos = mouseEvent->pos().x() / (float)slider_->width();
			setValue((pos * (max_ - min_) + min_));
		}
	}

	return QWidget::eventFilter(watched, event);
}
