#include "floatfield.h"

#include <QMouseEvent>
#include <QDoubleValidator>

FloatField::FloatField(QWidget* parent) : QLineEdit(parent)
	, dragging_(false), step_(1), value_(0) {
	updateText();

	setMouseTracking(true);
	setValidator(new QDoubleValidator(this));

	connect(this, SIGNAL(editingFinished()), this, SLOT(onEditingFinished()));
}

void FloatField::setValue(float value) {
	value_ = value;
	updateText();
}

void FloatField::updateText() {
	QString text = QString::number(value_, 'f', 2);
	int i = text.length() - 1;
	for (; text[i] != '.' && text[i] == '0'; --i)
		;
	text = text.left(i + int(text[i] != '.'));

	setText(text);
	emit valueChanged(value_);
}

void FloatField::mouseMoveEvent(QMouseEvent* e) {
	if (dragging_) {
		float delta = (e->pos() - pos_).x();
		value_ += step_ * delta;
		updateText();

		pos_ = e->pos();
	}

	QLineEdit::mouseMoveEvent(e);
}

void FloatField::mousePressEvent(QMouseEvent* e) {
	if (e->button() & Qt::MiddleButton) {
		dragging_ = true;
		pos_ = e->pos();
		setCursor(Qt::PointingHandCursor);
	}

	QLineEdit::mousePressEvent(e);
}

void FloatField::mouseReleaseEvent(QMouseEvent* e) {
	if (e->button() & Qt::MiddleButton) {
		dragging_ = false;
		setCursor(Qt::ArrowCursor);
	}
}

void FloatField::onEditingFinished() {
	value_ = text().toFloat();
	updateText();
}
