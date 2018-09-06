#include "intfield.h"

#include <QMouseEvent>
#include <QDoubleValidator>

#include "tools/math2.h"

IntField::IntField(QWidget* parent)
	: QLineEdit(parent)
	, dragging_(false), step_(1), value_(0), min_(INT_MIN), max_(INT_MAX) {
	updateText();

	setMouseTracking(true);
	connect(this, SIGNAL(editingFinished()), this, SLOT(onEditingFinished()));
}

void IntField::setValue(int value) {
	if (value != value_) {
		value_ = Math::Clamp(value, min_, max_);

		blockSignals(true);
		updateText();
		blockSignals(false);
	}
}

void IntField::updateText() {
	setText(QString::number(value_));
	emit valueChanged(value_);
}

void IntField::mouseMoveEvent(QMouseEvent* e) {
	if (dragging_) {
		float delta = (e->pos() - pos_).x();
		value_ += step_ * delta;
		updateText();

		pos_ = e->pos();
	}

	QLineEdit::mouseMoveEvent(e);
}

void IntField::mousePressEvent(QMouseEvent* e) {
	if (e->button() & Qt::MiddleButton) {
		dragging_ = true;
		pos_ = e->pos();
		setCursor(Qt::PointingHandCursor);
	}

	QLineEdit::mousePressEvent(e);
}

void IntField::mouseReleaseEvent(QMouseEvent* e) {
	if (e->button() & Qt::MiddleButton) {
		dragging_ = false;
		setCursor(Qt::ArrowCursor);
	}
}

void IntField::onEditingFinished() {
	bool ok = false;
	int f = text().toInt(&ok);
	if (ok) {
		value_ = Math::Clamp(f, min_, max_);
	}

	updateText();
}
