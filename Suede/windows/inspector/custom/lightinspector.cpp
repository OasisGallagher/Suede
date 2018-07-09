#include "lightinspector.h"
#include "windows/controls/colorpicker.h"

LightInspector::LightInspector(Object object) : CustomInspector("Light", object) {
	/*form_->addRow(tr("Color"), new LabelTexture)*/
	connect(ColorPicker::get(), SIGNAL(currentColorChanged(const QColor&)), this, SLOT(onColorPicked(const QColor&)));
}

void LightInspector::onColorPicked(const QColor& color) {

}
