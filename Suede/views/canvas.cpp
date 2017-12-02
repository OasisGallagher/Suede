#include "engine.h"
#include "canvas.h"

Canvas::Canvas(QWidget *parent) 
	: QGLWidget(parent) {
	updateTimer_ = startTimer(10);
}

Canvas::~Canvas() {
	killTimer(updateTimer_);
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
	emit repaint();
	Engine::Update();

	//QMetaObject::invokeMethod(this, "updateGL", Qt::QueuedConnection);
}

void Canvas::timerEvent(QTimerEvent *event) {
	if (event->timerId() == updateTimer_) {
		update();
	}
}
