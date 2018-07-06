#include <QEvent>
#include "colorpicker.h"

ColorPicker* instance_;

ColorPicker* ColorPicker::get() {
	if (instance_ == nullptr) {
		instance_ = new ColorPicker;
	}

	return instance_;
}

ColorPicker::ColorPicker() {
	setWindowTitle(tr("Select color"));
	setOption(QColorDialog::NoButtons);
	setFocusPolicy(Qt::StrongFocus);
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
