#include "lighting.h"

#include "world.h"
#include "graphics.h"
#include "ui_suede.h"

void Lighting::init(Ui::Suede* ui) {
	WinBase::init(ui);
	connect(ui_->ambient, SIGNAL(valueChanged(const QColor&)), this, SLOT(onAmbientChanged(const QColor&)));
	connect(ui_->occlusion, SIGNAL(stateChanged(int)), this, SLOT(onOcclusionChanged(int)));
	connect(ui_->fogColor, SIGNAL(valueChanged(const QColor&)), this, SLOT(onFogColorChanged(const QColor&)));
	connect(ui_->fogDensity, SIGNAL(valueChanged(float)), this, SLOT(onFogDensityChanged(float)));
}

Lighting::Lighting(QWidget* parent)
	: QDockWidget(parent) {
	setFeatures(AllDockWidgetFeatures);
}

void Lighting::showEvent(QShowEvent* event) {
	ui_->ambient->setValue(Environment::instance()->GetAmbientColor());
	ui_->occlusion->setChecked(Graphics::instance()->IsAmbientOcclusionEnabled());

	ui_->fogColor->setValue(Environment::instance()->GetFogColor());
	ui_->fogDensity->setValue(Environment::instance()->GetFogDensity());
}

void Lighting::onAmbientChanged(const QColor& color) {
	Environment::instance()->SetAmbientColor(glm::vec3(color.red(), color.green(), color.blue()) / 255.f);
}

void Lighting::onOcclusionChanged(int state) {
	Graphics::instance()->EnableAmbientOcclusion(!!state);
}

void Lighting::onFogColorChanged(const QColor& color) {
	Environment::instance()->SetFogColor(glm::vec3(color.red(), color.green(), color.blue()) / 255.f);
}

void Lighting::onFogDensityChanged(float density) {
	Environment::instance()->SetFogDensity(density);
}
