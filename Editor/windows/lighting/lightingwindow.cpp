#include "lightingwindow.h"

#include "world.h"
#include "graphics.h"
#include "ui_editor.h"

LightingWindow::LightingWindow(QWidget* parent) : ChildWindow(parent) {
	setFeatures(AllDockWidgetFeatures);
}

void LightingWindow::awake() {
	connect(ui_->ambient, SIGNAL(valueChanged(const QColor&)), this, SLOT(onAmbientChanged(const QColor&)));
	connect(ui_->occlusion, SIGNAL(stateChanged(int)), this, SLOT(onOcclusionChanged(int)));
	connect(ui_->fogColor, SIGNAL(valueChanged(const QColor&)), this, SLOT(onFogColorChanged(const QColor&)));
	connect(ui_->fogDensity, SIGNAL(valueChanged(float)), this, SLOT(onFogDensityChanged(float)));
}

void LightingWindow::showEvent(QShowEvent* event) {
	ui_->ambient->setValue(Environment::GetAmbientColor());
	ui_->occlusion->setChecked(Graphics::GetAmbientOcclusionEnabled());

	ui_->fogColor->setValue(Environment::GetFogColor());
	ui_->fogDensity->setValue(Environment::GetFogDensity());
}

void LightingWindow::onAmbientChanged(const QColor& color) {
	Environment::SetAmbientColor(Color(color.redF(), color.greenF(), color.blueF()));
}

void LightingWindow::onOcclusionChanged(int state) {
	Graphics::SetAmbientOcclusionEnabled(!!state);
}

void LightingWindow::onFogColorChanged(const QColor& color) {
	Environment::SetFogColor(Color(color.redF(), color.greenF(), color.blueF()));
}

void LightingWindow::onFogDensityChanged(float density) {
	Environment::SetFogDensity(density);
}
