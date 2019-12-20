#include "canvas.h"

#include <QResizeEvent>

Canvas::Canvas(QWidget *parent) : QGLWidget(parent) {
	glInit();
	setAutoBufferSwap(false);
}

void Canvas::resizeGL(int w, int h) {
	Q_ASSERT(0 && "resizeGL called on this platform");
	emit sizeChanged(w, h);
}
