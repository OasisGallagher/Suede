#pragma once
#include "color.h"
#include <QtOpenGL/QGLWidget>

class IMGUIWidget : public QGLWidget{
	Q_OBJECT

public:
	IMGUIWidget(QWidget* parent, QGLWidget* shareWidget);
	~IMGUIWidget();

public:
	void bind();
	void unbind();

	void setForegroundColor(const QColor& value);
	void setBackgroundColor(const QColor& value);

private:
	Color foreground_, background_;
	QGLContext* oldContext_ = nullptr;
};
