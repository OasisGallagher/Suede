#include "project.h"
#include "ui_editor.h"

#include <functional>

#include <QProcess>
#include <QMessageBox>
#include <QMouseEvent>
#include <QFileSystemModel>
#include <QDesktopServices>
#include <QFileIconProvider>

Project::Project(QWidget* parent) :QDockWidget(parent) {
}

void Project::init(Ui::Editor* ui) {
	WinBase::init(ui);
	ui->splitter->setStretchFactor(0, 1);
	ui->splitter->setStretchFactor(1, 4);

	ui->directoryTree->setRootPath("resources");

	connect(ui_->address, SIGNAL(editingFinished()), this, SLOT(onAddressChanged()));

	connect(ui_->listWidget, SIGNAL(itemClicked(QListWidgetItem*)), this, SLOT(onItemClicked(QListWidgetItem*)));
	connect(ui_->listWidget, SIGNAL(itemChanged(QListWidgetItem*)), this, SLOT(onItemChanged(QListWidgetItem*)));
	connect(ui_->listWidget, SIGNAL(itemDoubleClicked(QListWidgetItem*)), this, SLOT(onItemDoubleClicked(QListWidgetItem*)));
	connect(ui_->listWidget, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(onCustomContextMenu()));

	connect(ui_->directoryTree, SIGNAL(selectionChanged(const QStringList&)), this, SLOT(onSelectionChanged(const QStringList&)));
	connect(ui_->directoryTree, SIGNAL(requestContextMenuOnItems(const QStringList&)), this, SLOT(onCustomContextMenu()));
}

QString Project::newFolderName(const QString& parent) {
	QDir dir(parent);
	QString untitled = "New Folder";
	QSet<QString> set = QSet<QString>::fromList(dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot));
	if (!set.contains(untitled)) {
		return untitled;
	}

	for (int i = 1; ; ++i) {
		QString name = untitled + QString::number(i);
		if (!set.contains(name)) {
			return name;
		}
	}

	return "";
}

QStringList Project::selectedEntries(QWidget* sender) {
	QStringList entries;
	if (sender == ui_->listWidget) {
		for (QListWidgetItem* item : ui_->listWidget->selectedItems()) {
			entries.push_back(item->data(Qt::UserRole).toString());
		}
	}
	
	if (entries.empty() || sender == ui_->directoryTree) {
		entries = ui_->directoryTree->selectedDirectories();
	}

	return entries;
}

void Project::onAddressChanged() {
	ui_->directoryTree->selectDirectory(ui_->address->text());
}

void Project::onItemClicked(QListWidgetItem* item) {
}

void Project::onItemDoubleClicked(QListWidgetItem* item) {
	QFileInfo info = item->data(Qt::UserRole).toString();
	if (info.isDir()) {
		ui_->directoryTree->selectDirectory(item->data(Qt::UserRole).toString());
	}
	else {
		openEntries(QStringList() << info.filePath());
	}
}

void Project::onItemChanged(QListWidgetItem* item) {
	QString path = item->data(Qt::UserRole).toString();
	QFileInfo info(path);
	if (info.fileName() == item->text()) {
		return;
	}

	bool ok = false;
	if (info.isDir()) {
		ok = QDir(path).rename(info.path(), info.dir().path() + "/" + item->text());
	}
	else {
		ok = QFile(path).rename(info.dir().path() + "/" + item->text());
	}

	if (!ok) {
		item->setText(info.fileName());
	}
}

void Project::onCreateFolder(const QStringList& selected) {
	QString path = selected.front();
	QFileInfo info(path);

	if (!info.isDir()) {
		path = info.dir().path();
	}

	ui_->directoryTree->selectDirectory(path);

	QString name = newFolderName(path);
	if (QDir(path).mkdir(name)) {
		QFileIconProvider ip;
		QListWidgetItem* item = new QListWidgetItem(ip.icon(QFileIconProvider::Folder), name);
		ui_->listWidget->addItem(item);
		item->setFlags(item->flags() | Qt::ItemIsEditable);

		ui_->listWidget->sortItems();
		ui_->listWidget->editItem(item);
	}
}

void Project::onCreateEmptyShader(const QStringList& selected) {
	Debug::Log("onCreateEmptyShader");
}

void Project::onCreateImageEffectShader(const QStringList& selected) {
	Debug::Log("onCreateImageEffectShader");
}

void Project::onOpenSelected(const QStringList& selected) {
	openEntries(selected);
}

void Project::onDeleteSelected(const QStringList& selected) {
 	QString message("Are you sure to delete selected resources?");

	int limit = 5;
 	for (const QString& path : selected) {
		if (--limit < 0) {
			message += "\n...";
			break;
		}

 		message += "\n" + path;
 	}
 
 	if (QMessageBox::warning(this,
 		"Delete Selected Resources", message, QMessageBox::Yes, QMessageBox::No) == QMessageBox::Yes) {
		removeEntries(selected);
 	}
}

void Project::removeEntries(const QStringList& selected) {
	for (const QString& path : selected) {
		if (QFileInfo(path).isDir()) {
			QDir(path).removeRecursively();
		}
		else {
			QFile::remove(path);
		}
	}
}

void Project::onShowSelectedInExplorer(const QStringList& selected) {
 	QStringList args({ "/select,", "" });
 	QProcess* process = new QProcess(this);

 	for (const QString& path : selected) {
 		args.back() = QDir::toNativeSeparators(path);
 		process->start("explorer.exe", args);
 	}
}

void Project::onCustomContextMenu() {
	QMenu menu;

	QStringList selected = selectedEntries((QWidget*)sender());

	QMenu* create = menu.addMenu("Create");
	QAction* createFolder = new QAction("Folder", create);
	connect(createFolder, &QAction::triggered, this, std::bind(&Project::onCreateFolder, this, selected));
	create->addAction(createFolder);

	create->addSeparator();

	QMenu* createShader = create->addMenu("Shader");
	QAction* emptyShader = new QAction("Empty Shader", createShader);
	connect(emptyShader, &QAction::triggered, this, std::bind(&Project::onCreateEmptyShader, this, selected));

	QAction* imageEffectShader = new QAction("Image Effect Shader", createShader);
	connect(imageEffectShader, &QAction::triggered, this, std::bind(&Project::onCreateImageEffectShader, this, selected));

	createShader->addAction(emptyShader);
	createShader->addAction(imageEffectShader);

	QAction* showInExplorer = new QAction("Show In Explorer", &menu);
	connect(showInExplorer, &QAction::triggered, this, std::bind(&Project::onShowSelectedInExplorer, this, selected));

	QAction* open = new QAction("Open", &menu);
	connect(open, &QAction::triggered, this, std::bind(&Project::onOpenSelected, this, selected));

	QAction* del = new QAction("Delete", &menu);
	connect(del, &QAction::triggered, this, std::bind(&Project::onDeleteSelected, this, selected));

	menu.addAction(showInExplorer);
	menu.addAction(open);
	menu.addAction(del);
	menu.exec(QCursor::pos());
}

void Project::onSelectionChanged(const QStringList& directories) {
	if (!directories.empty()) {
		ui_->address->setText(directories.size() > 1 ? "Showing multiple folders..." : directories.front());
		ui_->address->setReadOnly(directories.size() > 1);
	}

	showContents(directories);
}

void Project::openEntries(const QStringList& entries) {
	for (const QString& entry : entries) {
		QDesktopServices::openUrl(QDir::currentPath() + "/" + entry);
	}
}

void Project::showContents(const QStringList& directories) {
	ui_->listWidget->clear();

	QSet<QString> set;
	QFileIconProvider ip;
	for (const QString& path : directories) {
		for (const QFileInfo& info : QDir(path).entryInfoList(QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot)) {
			if (set.contains(info.filePath())) {
				continue;
			}

			QListWidgetItem* item = new QListWidgetItem(ip.icon(info), info.fileName());
			item->setFlags(item->flags() | Qt::ItemIsEditable);
			ui_->listWidget->addItem(item);
			item->setData(Qt::UserRole, info.filePath());
			set.insert(info.filePath());
		}
	}
}
