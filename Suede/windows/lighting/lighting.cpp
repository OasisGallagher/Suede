#include "lighting.h"

#include "world.h"
#include "ui_suede.h"

void Lighting::init(Ui::Suede* ui) {
	WinBase::init(ui);
	connect(ui_->ambient, SIGNAL(valueChanged(const QColor&)), this, SLOT(onAmbientChanged(const QColor&)));

	connect(ui_->fogColor, SIGNAL(valueChanged(const QColor&)), this, SLOT(onFogColorChanged(const QColor&)));
	connect(ui_->fogDensity, SIGNAL(valueChanged(float)), this, SLOT(onFogDensityChanged(float)));
}

Lighting::Lighting(QWidget* parent)
	: QDockWidget(parent) {
	setFeatures(AllDockWidgetFeatures);
}

void Lighting::showEvent(QShowEvent* event) {
	ui_->ambient->setValue(World::get()->GetEnvironment()->GetAmbientColor());

	ui_->fogColor->setValue(World::get()->GetEnvironment()->GetFogColor());
	ui_->fogDensity->setValue(World::get()->GetEnvironment()->GetFogDensity());
}

void Lighting::onAmbientChanged(const QColor& color) {
	World::get()->GetEnvironment()->SetAmbientColor(glm::vec3(color.red(), color.green(), color.blue()) / 255.f);
}

void Lighting::onFogColorChanged(const QColor& color) {
	World::get()->GetEnvironment()->SetFogColor(glm::vec3(color.red(), color.green(), color.blue()) / 255.f);
}

void Lighting::onFogDensityChanged(float density) {
	World::get()->GetEnvironment()->SetFogDensity(density);
}
