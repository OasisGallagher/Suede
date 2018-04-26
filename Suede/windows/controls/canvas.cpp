#include "time2.h"
#include "engine.h"
#include "canvas.h"
#include "../windows/status/status.h"

#define FPS_REFRESH_TIME 0.5f

Canvas::Canvas(QWidget *parent) 
	: QGLWidget(parent), frameCounter_(0), timeCounter_(0), fps_(0) {
	glInit();
	makeCurrent();

	Engine::Initialize();
}

Canvas::~Canvas() {
	Engine::Release();
}

void Canvas::resizeGL(int w, int h) {
	Engine::Resize(w, h);
}

void Canvas::paintGL() {
	Engine::Update();
	UpdateFPS();
}

void Canvas::UpdateFPS() {
	if (timeCounter_ < FPS_REFRESH_TIME) {
		timeCounter_ += Time::GetDeltaTime();
		frameCounter_++;
	}
	else {
		fps_ = (float)frameCounter_ / timeCounter_;
		frameCounter_ = 0;
		timeCounter_ = 0.0f;
		Status::get()->updateFPS(fps_);
	}
}
