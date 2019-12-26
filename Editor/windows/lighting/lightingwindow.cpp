#include "lightingwindow.h"

#include "ui_editor.h"

#include "engine.h"
#include "scene.h"
#include "graphics.h"

LightingWindow::LightingWindow(QWidget* parent) : ChildWindow(parent) {
	setFeatures(AllDockWidgetFeatures);
}

void LightingWindow::initUI() {
	connect(ui_->ambient, SIGNAL(valueChanged(const QColor&)), this, SLOT(onAmbientChanged(const QColor&)));
	connect(ui_->occlusion, SIGNAL(stateChanged(int)), this, SLOT(onOcclusionChanged(int)));
	connect(ui_->fogColor, SIGNAL(valueChanged(const QColor&)), this, SLOT(onFogColorChanged(const QColor&)));
	connect(ui_->fogDensity, SIGNAL(valueChanged(float)), this, SLOT(onFogDensityChanged(float)));
}

void LightingWindow::showEvent(QShowEvent* event) {
	Environment* env = Engine::GetSubsystem<Scene>()->GetEnvironment();
	ui_->ambient->setValue(env->ambientColor);
	ui_->occlusion->setChecked(Engine::GetSubsystem<Graphics>()->GetAmbientOcclusionEnabled());

	ui_->fogColor->setValue(env->fogColor);
	ui_->fogDensity->setValue(env->fogDensity);
}

void LightingWindow::onAmbientChanged(const QColor& color) {
	Engine::GetSubsystem<Scene>()->GetEnvironment()->ambientColor.Set(color.redF(), color.greenF(), color.blueF());
}

void LightingWindow::onOcclusionChanged(int state) {
	Engine::GetSubsystem<Graphics>()->SetAmbientOcclusionEnabled(!!state);
}

void LightingWindow::onFogColorChanged(const QColor& color) {
	Engine::GetSubsystem<Scene>()->GetEnvironment()->fogColor.Set(color.redF(), color.greenF(), color.blueF());
}

void LightingWindow::onFogDensityChanged(float density) {
	Engine::GetSubsystem<Scene>()->GetEnvironment()->fogDensity = density;
}
