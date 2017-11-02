#include "views/window.h"

Window::Window() : view_(nullptr) {
}

void Window::setView(View* widget) {
	view_ = widget;
	initialize();
}
