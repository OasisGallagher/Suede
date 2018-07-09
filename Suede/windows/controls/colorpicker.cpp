#include <QMouseEvent>
#include "colorpicker.h"

ColorPicker* instance_;

void ColorPicker::mousePressEvent(QMouseEvent* event) {
	QPoint p = event->pos();
	if (p.x() < 0 || p.x() > width() || p.y() < 0 || p.y() > height()) {
		close();
	}
	else {
		pos_ = p;
	}
}

void ColorPicker::mouseMoveEvent(QMouseEvent* event) {
	if (event->buttons() & Qt::LeftButton) {
		QPoint diff = event->pos() - pos_;
		move(pos() + diff);
	}
}

ColorPicker* ColorPicker::get() {
	if (instance_ == nullptr) {
		instance_ = new ColorPicker;
	}

	return instance_;
}

ColorPicker::ColorPicker() {
	setWindowTitle(tr("Select color"));

	setWindowFlags(Qt::Popup);
	setOption(QColorDialog::NoButtons);
}

void ColorPicker::popup() {
	instance_->show();
	//instance_->setFocus();
}

void ColorPicker::destroy() {
	delete instance_;
	instance_ = nullptr;
}

void ColorPicker::focusOutEvent(QFocusEvent* event) {
//	instance_->close();
}
