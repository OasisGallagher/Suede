#include "imguiwidget.h"

#include <QWidget>
#include <QLayout>

#include "gui.h"
#include "gui/qtimgui/QtImGui.h"

IMGUIWidget::IMGUIWidget(QWidget* parent) : QGLWidget(parent) {
	QtImGui::create(this);
	GUI::LoadFont("resources/fonts/tahoma.ttf");
}

IMGUIWidget::~IMGUIWidget() {
	QtImGui::destroy(this);
}

void IMGUIWidget::bind() {
	oldContext_ = (QGLContext*)QGLContext::currentContext();
	makeCurrent();

	QtImGui::newFrame(this);

	GUI::Begin(width(), height(), foreground_, background_);
}

void IMGUIWidget::unbind() {
	GUI::End();

	swapBuffers();
	doneCurrent();

	if (oldContext_ != nullptr) {
		oldContext_->makeCurrent();
		oldContext_ = nullptr;
	}
}

void IMGUIWidget::setForegroundColor(const QColor& value) {
	foreground_.r = value.redF();
	foreground_.g = value.greenF();
	foreground_.b = value.blueF();
}

void IMGUIWidget::setBackgroundColor(const QColor& value) {
	background_.r = value.redF();
	background_.g = value.greenF();
	background_.b = value.blueF();
}
