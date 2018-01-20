#include "engine.h"
#include "canvas.h"

Canvas::Canvas(QWidget *parent) 
	: QGLWidget(parent) {
}

Canvas::~Canvas() {
	Engine::Release();
}

void Canvas::initializeGL() {
	// TODO: make debug context.
	Engine::Initialize();
}

void Canvas::resizeGL(int w, int h) {
	Engine::Resize(w, h);
}

void Canvas::paintGL() {
	Engine::Update();
}
