#pragma once
#include <QTimerEvent>
#include <QtOpenGL/QGLWidget>

class Canvas : public QGLWidget {
	Q_OBJECT

public:
	Canvas(QWidget *parent = NULL);
	~Canvas();

public:
	void redraw();

protected:
	virtual void resizeGL(int w, int h);
};
