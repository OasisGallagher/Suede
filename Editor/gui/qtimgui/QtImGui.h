#pragma once
class QGLWidget;

namespace QtImGui {

void create(QGLWidget *window);
void newFrame(QGLWidget* widget);
void destroy(QGLWidget* widget);
void destroyAll();

}
