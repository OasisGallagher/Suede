#include "time2.h"
#include "engine.h"
#include "canvas.h"
#include "../windows/status/status.h"

#include "graphicsviewer.h"

#include "screen.h"

Canvas::Canvas(QWidget *parent) : QGLWidget(parent) {
	glInit();
	setAutoBufferSwap(false);
}

Canvas::~Canvas() {
}

void Canvas::resizeGL(int w, int h) {
	emit sizeChanged(w, h);
}
