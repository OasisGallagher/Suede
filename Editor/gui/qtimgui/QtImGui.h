#pragma once
class QWidget;
class QWindow;

// SUEDE TODO: multiple render context...
namespace QtImGui {

void initialize(QWidget *window);
void newFrame(QWidget* widget);
void destroy(QWidget* widget);
void destroyAll();

}
