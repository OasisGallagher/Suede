#pragma once
#include <QtOpenGL/QGLWidget>
#include "graphicscanvas.h"

class Canvas : public QGLWidget, public GraphicsCanvas {
	Q_OBJECT

public:
	Canvas(QWidget *parent = NULL);
	~Canvas() {}

protected:
	virtual void paintGL() {}
	virtual void resizeGL(int w, int h);

protected:
	virtual void MakeCurrent() { makeCurrent(); }
	virtual void DoneCurrent() { doneCurrent(); }
	virtual void SwapBuffers() { swapBuffers(); }

	virtual uint GetWidth() { return width(); }
	virtual uint GetHeight() { return height(); }
};
