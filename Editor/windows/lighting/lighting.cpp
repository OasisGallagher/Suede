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
	ui_->ambient->setValue(Environment::GetAmbientColor());
	ui_->occlusion->setChecked(Graphics::GetAmbientOcclusionEnabled());

	ui_->fogColor->setValue(Environment::GetFogColor());
	ui_->fogDensity->setValue(Environment::GetFogDensity());
}

void Lighting::onAmbientChanged(const QColor& color) {
	Environment::SetAmbientColor(Color(color.redF(), color.greenF(), color.blueF()));
}

void Lighting::onOcclusionChanged(int state) {
	Graphics::SetAmbientOcclusionEnabled(!!state);
}

void Lighting::onFogColorChanged(const QColor& color) {
	Environment::SetFogColor(Color(color.redF(), color.greenF(), color.blueF()));
}

void Lighting::onFogDensityChanged(float density) {
	Environment::SetFogDensity(density);
}
