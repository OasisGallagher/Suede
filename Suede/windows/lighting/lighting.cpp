#include "lighting.h"

#include "world.h"
#include "ui_suede.h"

void Lighting::init(Ui::Suede* ui) {
	WinSingleton::init(ui);
}

Lighting::Lighting(QWidget* parent)
	: QDockWidget(parent) {
	setFeatures(AllDockWidgetFeatures);
	connect(ui_->ambient, SIGNAL(valueChanged(const QColor&)), this, SLOT(onAmbientChanged(const QColor&)));
}

void Lighting::showEvent(QShowEvent* event) {
	ui_->ambient->setValue(WorldInstance()->GetEnvironment()->GetAmbientColor());
}

void Lighting::onAmbientChanged(const QColor& color) {
	WorldInstance()->GetEnvironment()->SetAmbientColor(glm::vec3(color.red(), color.green(), color.blue()) / 255.f);
}
