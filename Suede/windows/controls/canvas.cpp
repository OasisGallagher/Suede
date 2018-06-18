#include "canvas.h"

Canvas::Canvas(QWidget *parent) : QGLWidget(parent) {
	glInit();
	setAutoBufferSwap(false);
}

void Canvas::resizeGL(int w, int h) {
	emit sizeChanged(w, h);
}
