#include <QLineEdit>
#include <QMouseEvent>

#include "os/filesystem.h"
#include "filetreefield.h"

FileTreeField::FileTreeField(QWidget* parent) :QComboBox(parent), menu_(nullptr) {
	setEditable(true);
	lineEdit()->setReadOnly(true);
	lineEdit()->setAlignment(Qt::AlignLeft);
	lineEdit()->installEventFilter(this);
}

FileTreeField::~FileTreeField() {
	delete menu_;
}

void FileTreeField::setDirectory(const QString& directory, const QString& selected, const QString& regex) {
	regex_ = regex;
	directory_ = directory;
	setEditText(selected);
}

void FileTreeField::showPopup() {
	QComboBox::showPopup();

	FileTree tree;
	if (!FileSystem::ListFileTree(tree, directory_.toStdString(), regex_.toStdString())) {
		hidePopup();
	}
	else {
		if (menu_ != nullptr) {
			delete menu_;
		}

		createMenu(tree);
	}
}

void FileTreeField::createMenu(FileTree &tree) {
	menu_ = new QMenu(this);
	connect(menu_, SIGNAL(aboutToHide()), this, SLOT(onHideMenu()));

	createSubMenu(menu_, tree.GetRoot());
	menu_->exec(mapToGlobal(QPoint(0, geometry().height())));
}

void FileTreeField::hidePopup() {
	QComboBox::hidePopup();
}

bool FileTreeField::eventFilter(QObject *watched, QEvent *event) {
	if (lineEdit() == watched && event->type() == QEvent::MouseButtonPress && ((QMouseEvent*)event)->buttons() & Qt::LeftButton) {
		showPopup();
	}
	
	return QComboBox::eventFilter(watched, event);
}

void FileTreeField::onHideMenu() {
	hidePopup();
}

void FileTreeField::onSelectItem() {
	QString path = ((QAction*)sender())->data().toString();
	emit selectionChanged(path);

	setEditText(FileSystem::GetFileNameWithoutExtension(path.toStdString()).c_str());
}

void FileTreeField::createSubMenu(QMenu* parent, FileEntry* entry) {
	for (uint i = 0; i < entry->GetChildCount(); ++i) {
		FileEntry* child = entry->GetChildAt(i);
		QString path = child->GetPath().c_str();
		QString name = FileSystem::GetFileNameWithoutExtension(child->GetPath()).c_str();
		if (child->IsDirectory()) {
			QMenu* subMenu = parent->addMenu(name);
			createSubMenu(subMenu, child);
		}
		else {
			QAction* action = parent->addAction(name);
			action->setData(path);
			connect(action, SIGNAL(triggered()), this, SLOT(onSelectItem()));
		}
	}
}
