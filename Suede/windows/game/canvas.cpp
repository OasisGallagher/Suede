#include <QSurfaceFormat>
#include "canvas.h"

static bool glInitCalled = false;
Canvas::Canvas(QWidget *parent) : QGLWidget(parent) {
	if (!glInitCalled) {
		glInit();
		setAutoBufferSwap(false);
		glInitCalled = true;
	}
}

void Canvas::resizeGL(int w, int h) {
	emit sizeChanged(w, h);
}
