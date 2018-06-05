#include <QMouseEvent>

#include "floatlabel.h"
#include "debug/debug.h"
#include "tools/math2.h"

FloatLabel::FloatLabel(QWidget* parent) : QLabel(parent)
	, dragging_(false), precision_(2), step_(0.5f), float_(0) {
	setMouseTracking(true);
}

void FloatLabel::updateText(float f) {
	((QLineEdit*)buddy())->setText(QString::number(f, 'g', precision_));
}

void FloatLabel::setPrecision(uint value) {
	if (!Math::Approximately(precision_, value)) {
		precision_ = value;
		updateText(float_);
	}
}

void FloatLabel::mousePressEvent(QMouseEvent* ev) {
	if (ev->button() & Qt::LeftButton) {
		dragging_ = true;
		pos_ = ev->pos();
		setCursor(Qt::PointingHandCursor);
	}

	QLabel::mousePressEvent(ev);
}

void FloatLabel::mouseReleaseEvent(QMouseEvent* ev) {
	if (ev->button() & Qt::LeftButton) {
		dragging_ = false;
		setCursor(Qt::ArrowCursor);
	}

	QLabel::mouseReleaseEvent(ev);
}

void FloatLabel::mouseMoveEvent(QMouseEvent* ev) {
	if (dragging_) {
		float delta = (ev->pos() - pos_).x();
		updateText(float_ += step_ * delta);

		pos_ = ev->pos();
	}
}
