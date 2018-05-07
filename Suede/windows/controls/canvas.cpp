#include "time2.h"
#include "engine.h"
#include "canvas.h"
#include "../windows/status/status.h"

Canvas::Canvas(QWidget *parent) : QGLWidget(parent) {
	glInit();
	makeCurrent();
	// TODO: switch GL context.
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
}

