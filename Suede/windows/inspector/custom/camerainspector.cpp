#include <QGroupBox>
#include <QFormLayout>

#include "tools/math2.h"
#include "../../controls/floatslider.h"

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
	slider->setValue(Math::Degrees(dsp_cast<Camera>(object)->GetFieldOfView()));

	connect(slider, SIGNAL(valueChanged(const QString&, float)), this, SLOT(onSliderValueChanged(const QString&, float)));

	form_->addRow(formatRowName("Fov"), slider);
}

void CameraInspector::onSliderValueChanged(const QString& name, float value) {
	if (name == Literals::cameraFovSlider) {
		dsp_cast<Camera>(object_)->SetFieldOfView(Math::Radians(float(value)));
	}
	else if (name == Literals::cameraFovSlider2) {
		dsp_cast<Camera>(object_)->SetOrthographicSize(value);
	}
}
