#include "colorpicker.h"
ColorPicker* colorPickerInstance;

ColorPicker* ColorPicker::get() {
	if (colorPickerInstance == nullptr) {
		colorPickerInstance = new ColorPicker;
		colorPickerInstance->setWindowTitle(tr("Select color"));
		colorPickerInstance->setOption(QColorDialog::NoButtons);
	}

	return colorPickerInstance;
}

void ColorPicker::destroy() {
	delete colorPickerInstance;
	colorPickerInstance = nullptr;
}
