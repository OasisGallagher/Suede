#include "light.h"

#include "lightinspector.h"
#include "windows/controls/colorfield.h"

LightInspector::LightInspector(Object object) : CustomInspector("Light", object) {
	ColorField* colorField = new ColorField(this);

	Light light = suede_dynamic_cast<Light>(target_);
	colorField->setColor(light->GetColor());

	form_->addRow(formatRowName("Color"), colorField);
	connect(colorField, SIGNAL(currentColorChanged(const QColor&)), this, SLOT(onCurrentColorChanged(const QColor&)));
}

void LightInspector::onCurrentColorChanged(const QColor& color) {
	Light light = suede_dynamic_cast<Light>(target_);
	light->SetColor(glm::vec3(color.red(), color.green(), color.blue()) / 255.f);
}
