#include "folddedcombobox.h"

void FolddedComboBox::setItems(const QStringList& items) {

}

void FolddedComboBox::showPopup() {
	QMenu* menu = new QMenu(this);
	QMenu* subMenu = new QMenu("sub");
	menu->addMenu(subMenu);
	subMenu->addAction("item");
	menu->exec(this->mapToGlobal(QPoint(0, geometry().height())));
}

void FolddedComboBox::hidePopup() {

}
