#include "engine.h"
#include "canvas.h"

Canvas::Canvas(QWidget *parent) 
	: QGLWidget(parent) {
	updateTimer_ = startTimer(10);
}

Canvas::~Canvas() {
	killTimer(updateTimer_);
	Engine::get()->release();
}

void Canvas::initializeGL() {
	// TODO: make debug context.
	Engine::get()->initialize();
}

void Canvas::resizeGL(int w, int h) {
	Engine::get()->resize(w, h);
}

void Canvas::paintGL() {
	emit repaint();
	Engine::get()->update();

	//QMetaObject::invokeMethod(this, "updateGL", Qt::QueuedConnection);
}

void Canvas::timerEvent(QTimerEvent *event) {
	if (event->timerId() == updateTimer_) {
		update();
	}
}
