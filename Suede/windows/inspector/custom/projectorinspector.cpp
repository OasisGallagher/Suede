#include <QGroupBox>

#include "tools/math2.h"
#include "widgets/fields/rangefield.h"

#include "projector.h"
#include "projectorinspector.h"

namespace Constants {
	static uint minFieldOfView = 0;
	static uint maxFieldOfView = 180;

	static uint minOrthographicSize = 1;
	static uint maxOrthographicSize = 20;
}

ProjectorInspector::ProjectorInspector(Object object) : CustomInspector("Projector", object) {
	RangeField* slider = new RangeField(this);

	Projector projector = suede_dynamic_cast<Projector>(target_);

	if (projector->GetPerspective()) {
		slider->setRange(Constants::minFieldOfView, Constants::maxFieldOfView);
		slider->setValue(Math::Degrees(projector->GetFieldOfView()));
		form_->addRow(formatRowName("Fov"), slider);
	}
	else {
		slider->setRange(Constants::minOrthographicSize, Constants::maxOrthographicSize);
		slider->setValue(projector->GetOrthographicSize());
		form_->addRow(formatRowName("Size"), slider);
	}

	connect(slider, SIGNAL(valueChanged(float)), this, SLOT(onSliderValueChanged(float)));
}

void ProjectorInspector::onSliderValueChanged(float value) {
	Projector projector = suede_dynamic_cast<Projector>(target_);
	if (projector->GetPerspective()) {
		projector->SetFieldOfView(Math::Radians(float(value)));
	}
	else {
		projector->SetOrthographicSize(value);
	}
}
