#pragma once
#include <QtOpenGL/QGLWidget>

class Canvas : public QGLWidget {
	Q_OBJECT

public:
	Canvas(QWidget *parent = NULL);
	~Canvas();

protected:
	virtual void paintGL();
	virtual void resizeGL(int w, int h);
};
