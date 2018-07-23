#include "light.h"

#include "lightinspector.h"
#include "widgets/controls/colorfield.h"
#include "widgets/controls/floatfield.h"

LightInspector::LightInspector(Object object) : CustomInspector("Light", object) {
	Light light = suede_dynamic_cast<Light>(target_);

	ColorField* colorField = new ColorField(this);
	colorField->setValue(light->GetColor());

	form_->addRow(formatRowName("Color"), colorField);
	connect(colorField, SIGNAL(valueChanged(const QColor&)), this, SLOT(onCurrentColorChanged(const QColor&)));

	FloatField* intensityField = new FloatField(this);
	intensityField->setValue(light->GetIntensity());

	form_->addRow(formatRowName("Intensity"), intensityField);
	connect(intensityField, SIGNAL(valueChanged(float)), this, SLOT(onIntensityChanged(float)));
}

void LightInspector::onIntensityChanged(float intensity) {
	Light light = suede_dynamic_cast<Light>(target_);
	light->SetIntensity(intensity);
}

void LightInspector::onCurrentColorChanged(const QColor& color) {
	Light light = suede_dynamic_cast<Light>(target_);
	light->SetColor(glm::vec3(color.red(), color.green(), color.blue()) / 255.f);
}
