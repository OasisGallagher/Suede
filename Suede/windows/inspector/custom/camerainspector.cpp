#include <QGroupBox>

#include "tools/math2.h"
#include "windows/controls/enumfield.h"
#include "windows/controls/vec4field.h"
#include "windows/controls/floatfield.h"
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
	Camera camera = suede_dynamic_cast<Camera>(object);

	slider->setObjectName(Literals::cameraFovSlider);
	slider->setRange(Constants::minFieldOfView, Constants::maxFieldOfView);
	slider->setValue(Math::Degrees(camera->GetFieldOfView()));

	connect(slider, SIGNAL(valueChanged(const QString&, float)), this, SLOT(onSliderValueChanged(const QString&, float)));

	form_->addRow(formatRowName("Fov"), slider);

	EnumField* clearType = new EnumField(this);
	clearType->setEnums(suede_dynamic_cast<Camera>(target_)->GetClearType());
	connect(clearType, SIGNAL(currentIndexChanged(const QString&)), this, SLOT(onClearTypeChanged(const QString&)));
	form_->addRow(formatRowName("ClearType"), clearType);

	FloatField* nearClipPlane = new FloatField(this);
	form_->addRow(formatRowName("Near"), nearClipPlane);
	
	Vec4Field* rect = new Vec4Field(this, QStringList({ "X", "Y", "W", "H" }));
	form_->addRow(formatRowName("Rect"), rect);

	const Rect& r = camera->GetRect();
	glm::vec4 v4(r.GetXMin(), r.GetYMin(), r.GetWidth(), r.GetHeight());
	rect->setValue(v4);
}

void CameraInspector::onClearTypeChanged(const QString& text) {
	ClearType type = ClearType::from_string(text.toLatin1());
	suede_dynamic_cast<Camera>(target_)->SetClearType(type);
}

void CameraInspector::onSliderValueChanged(const QString& name, float value) {
	if (name == Literals::cameraFovSlider) {
		suede_dynamic_cast<Camera>(target_)->SetFieldOfView(Math::Radians(float(value)));
	}
	else if (name == Literals::cameraFovSlider2) {
		suede_dynamic_cast<Camera>(target_)->SetOrthographicSize(value);
	}
}
