#pragma once
#include <QTimerEvent>
#include <QtOpenGL/QGLWidget>

class Canvas : public QGLWidget {
	Q_OBJECT

public:
	Canvas(QWidget *parent = NULL);
	~Canvas();

protected:
	void initializeGL();
	void resizeGL(int w, int h);
	void paintGL();

	void timerEvent(QTimerEvent *event);

signals:
	void repaint();

private:
	int updateTimer_;
};
