#include "engine.h"
#include "canvas.h"

Canvas::Canvas(QWidget *parent) 
	: QGLWidget(parent) {
	glInit();
	Engine::Initialize();
}

Canvas::~Canvas() {
	Engine::Release();
}

void Canvas::resizeGL(int w, int h) {
	Engine::Resize(w, h);
}

void Canvas::redraw() {
	makeCurrent();

	Engine::Update();
	swapBuffers();
	doneCurrent();
}
