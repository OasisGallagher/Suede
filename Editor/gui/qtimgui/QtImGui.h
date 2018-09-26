#pragma once
class QGLWidget;

// SUEDE TODO: multiple render context...
namespace QtImGui {

void create(QGLWidget *window);
void newFrame(QGLWidget* widget);
void destroy(QGLWidget* widget);
void destroyAll();

}
