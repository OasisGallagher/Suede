#pragma once
#include <QtOpenGL/QGLWidget>
#include "glcanvas.h"

class Canvas : public QGLWidget, public GLCanvas {
	Q_OBJECT

public:
	Canvas(QWidget *parent = NULL);
	~Canvas() {}

protected:
	virtual void MakeCurrent() { makeCurrent(); }
	virtual void DoneCurrent() { doneCurrent(); }
	virtual void SwapBuffers() { swapBuffers(); }

	virtual uint GetWidth() { return width(); }
	virtual uint GetHeight() { return height(); }
};
