#include <QGroupBox>
#include <QFormLayout>

#include "tools/math2.h"
#include "../../controls/floatslider.h"

#include "projector.h"
#include "projectorinspector.h"

namespace Constants {
	static uint minFieldOfView = 0;
	static uint maxFieldOfView = 180;

	static uint minOrthographicSize = 1;
	static uint maxOrthographicSize = 20;
}

namespace Literals {
	DEFINE_LITERAL(projectorFovSlider);
	DEFINE_LITERAL(projectorFovSlider2);
}

ProjectorInspector::ProjectorInspector(Object object) : CustomInspector("Projector", object) {
	FloatSlider* slider = new FloatSlider(this);

	Projector projector = dsp_cast<Projector>(object_);

	if (projector->GetPerspective()) {
		slider->setObjectName(Literals::projectorFovSlider);
		slider->setRange(Constants::minFieldOfView, Constants::maxFieldOfView);
		slider->setValue(Math::Degrees(projector->GetFieldOfView()));
		form_->addRow(formatRowName("Fov"), slider);
	}
	else {
		slider->setObjectName(Literals::projectorFovSlider2);
		slider->setRange(Constants::minOrthographicSize, Constants::maxOrthographicSize);
		slider->setValue(projector->GetOrthographicSize());
		form_->addRow(formatRowName("Size"), slider);
	}

	connect(slider, SIGNAL(valueChanged(const QString&, float)), this, SLOT(onSliderValueChanged(const QString&, float)));
}

void ProjectorInspector::onSliderValueChanged(const QString& name, float value) {
	if (name == Literals::projectorFovSlider) {
		dsp_cast<Projector>(object_)->SetFieldOfView(Math::Radians(float(value)));
	}
	else if (name == Literals::projectorFovSlider2) {
		dsp_cast<Projector>(object_)->SetOrthographicSize(value);
	}
}
