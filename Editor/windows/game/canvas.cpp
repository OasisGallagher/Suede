#include "canvas.h"

#include <QResizeEvent>

Canvas::Canvas(QWidget *parent) : QGLWidget(parent) {
	glInit();
	setAutoBufferSwap(false);
}
