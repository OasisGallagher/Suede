#include "lighting.h"

#include "world.h"
#include "graphics.h"
#include "ui_editor.h"

void Lighting::init(Ui::Editor* ui) {
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
	ui_->occlusion->setChecked(Graphics::instance()->GetAmbientOcclusionEnabled());

	ui_->fogColor->setValue(Environment::instance()->GetFogColor());
	ui_->fogDensity->setValue(Environment::instance()->GetFogDensity());
}

void Lighting::onAmbientChanged(const QColor& color) {
	Environment::instance()->SetAmbientColor(glm::vec3(color.redF(), color.greenF(), color.blueF()));
}

void Lighting::onOcclusionChanged(int state) {
	Graphics::instance()->SetAmbientOcclusionEnabled(!!state);
}

void Lighting::onFogColorChanged(const QColor& color) {
	Environment::instance()->SetFogColor(glm::vec3(color.redF(), color.greenF(), color.blueF()));
}

void Lighting::onFogDensityChanged(float density) {
	Environment::instance()->SetFogDensity(density);
}
