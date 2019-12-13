#include "childwindow.h"

#include "editor.h"

ChildWindow::ChildWindow(QWidget* parent) {
	editor_ = dynamic_cast<Editor*>(parent);
	ui_ = editor_->ui();
}
