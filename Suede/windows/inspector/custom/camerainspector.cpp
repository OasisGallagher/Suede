#include <QGroupBox>

#include "tools/math2.h"
#include "windows/controls/enumfield.h"
#include "windows/controls/floatslider.h"

#include "camera.h"
#include "camerainspector.h"

namespace Constants {
	static uint minFieldOfView = 0;
	static uint maxFieldOfView = 180;
}
namespace Literals {
	DEFINE_LITERAL(cameraFovLabel);
	DEFINE_LITERAL(cameraFovSlider);
	DEFINE_LITERAL(cameraFovSlider2);
}

CameraInspector::CameraInspector(Object object) : CustomInspector("Camera", object) {
	FloatSlider* slider = new FloatSlider(this);
	slider->setObjectName(Literals::cameraFovSlider);
	slider->setRange(Constants::minFieldOfView, Constants::maxFieldOfView);
	slider->setValue(Math::Degrees(suede_dynamic_cast<Camera>(object)->GetFieldOfView()));

	connect(slider, SIGNAL(valueChanged(const QString&, float)), this, SLOT(onSliderValueChanged(const QString&, float)));

	form_->addRow(formatRowName("Fov"), slider);

	EnumField* field = new EnumField(this);
	field->setEnums(+ClearType::Skybox);
	form_->addRow(formatRowName("ClearType"), field);
}

void CameraInspector::onSliderValueChanged(const QString& name, float value) {
	if (name == Literals::cameraFovSlider) {
		suede_dynamic_cast<Camera>(target_)->SetFieldOfView(Math::Radians(float(value)));
	}
	else if (name == Literals::cameraFovSlider2) {
		suede_dynamic_cast<Camera>(target_)->SetOrthographicSize(value);
	}
}
