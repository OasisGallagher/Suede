#include "lighting.h"

#include "world.h"
#include "ui_suede.h"

void Lighting::init(Ui::Suede* ui) {
	WinBase::init(ui);
	connect(ui_->ambient, SIGNAL(valueChanged(const QColor&)), this, SLOT(onAmbientChanged(const QColor&)));
}

Lighting::Lighting(QWidget* parent)
	: QDockWidget(parent) {
	setFeatures(AllDockWidgetFeatures);
}

void Lighting::showEvent(QShowEvent* event) {
	ui_->ambient->setValue(WorldInstance()->GetEnvironment()->GetAmbientColor());
}

void Lighting::onAmbientChanged(const QColor& color) {
	WorldInstance()->GetEnvironment()->SetAmbientColor(glm::vec3(color.red(), color.green(), color.blue()) / 255.f);
}
