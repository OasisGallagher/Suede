#pragma once
#include <QTimerEvent>
#include <QtOpenGL/QGLWidget>

class Canvas : public QGLWidget {
	Q_OBJECT

public:
	Canvas(QWidget *parent = NULL);
	~Canvas();

protected:
	virtual void initializeGL();
	virtual void resizeGL(int w, int h);
	virtual void paintGL();
};
