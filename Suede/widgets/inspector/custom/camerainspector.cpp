#include <QGroupBox>

#include "camera.h"
#include "camerainspector.h"

#include "tools/math2.h"

#include "widgets/controls/enumfield.h"
#include "widgets/controls/vec4field.h"
#include "widgets/controls/rectfield.h"
#include "widgets/controls/colorfield.h"
#include "widgets/controls/floatfield.h"
#include "widgets/controls/floatslider.h"

namespace Constants {
	static uint minFieldOfView = 0;
	static uint maxFieldOfView = 180;
}

namespace Literals {
	DEFINE_LITERAL(cameraFovLabel);
	DEFINE_LITERAL(cameraFovSlider);
	DEFINE_LITERAL(cameraFovSlider2);
}

CameraInspector::CameraInspector(Object object) : CustomInspector("Camera", object), clearColorField_(nullptr) {
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

	clearTypeRow_ = form_->rowCount();
	form_->addRow(formatRowName("ClearType"), clearType);

	FloatField* nearClipPlane = new FloatField(this);
	form_->addRow(formatRowName("Near"), nearClipPlane);
	
	RectField* rect = new RectField(this);
	form_->addRow(formatRowName("Rect"), rect);
	connect(rect, SIGNAL(valueChanged(const Rect&)), this, SLOT(onRectChanged(const Rect&)));

	rect->setValue(camera->GetRect());
}

void CameraInspector::onRectChanged(const Rect& rect) {
	suede_dynamic_cast<Camera>(target_)->SetRect(rect);
}

void CameraInspector::onClearTypeChanged(const QString& text) {
	ClearType type = ClearType::from_string(text.toLatin1());
	suede_dynamic_cast<Camera>(target_)->SetClearType(type);
	if (type == ClearType::Color) {
		ColorField* colorField = new ColorField(this);
		colorField->setValue(suede_dynamic_cast<Camera>(target_)->GetClearColor());
		connect(colorField, SIGNAL(valueChanged(const QColor&)), this, SLOT(onClearColorChanged(const QColor&)));
		form_->insertRow(clearTypeRow_ + 1, formatRowName("ClearColor"), colorField);
	}
}

void CameraInspector::onClearColorChanged(const QColor& color) {
	suede_dynamic_cast<Camera>(target_)->SetClearColor(glm::vec3(color.red(), color.blue(), color.green()) / 255.f);
}

void CameraInspector::onSliderValueChanged(const QString& name, float value) {
	if (name == Literals::cameraFovSlider) {
		suede_dynamic_cast<Camera>(target_)->SetFieldOfView(Math::Radians(float(value)));
	}
	else if (name == Literals::cameraFovSlider2) {
		suede_dynamic_cast<Camera>(target_)->SetOrthographicSize(value);
	}
}
