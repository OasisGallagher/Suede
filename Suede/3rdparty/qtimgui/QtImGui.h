#pragma once
#include "imgui/imgui.h"

class QWidget;
class QWindow;

// TODO: multiple render context...
namespace QtImGui {

#ifdef QT_WIDGETS_LIB
void initialize(QWidget *window);
#endif

void initialize(QWindow *window);
void newFrame();
void destroy();
}
