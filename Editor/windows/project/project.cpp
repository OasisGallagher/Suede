#include "project.h"
#include "ui_editor.h"

#include <QProcess>
#include <QMessageBox>
#include <QMouseEvent>
#include <QFileSystemModel>
#include <QDesktopServices>

Project::Project(QWidget* parent) :QDockWidget(parent) {
}

void Project::init(Ui::Editor* ui) {
	WinBase::init(ui);

	model_ = new QFileSystemModel(this);
	model_->setFilter(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot);
	
	ui->fileTree->setModel(model_);

	ui->fileTree->setRootIndex(model_->setRootPath("resources"));

	ui->fileTree->setHeaderHidden(true);

	connect(ui_->fileTree->selectionModel(), SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)),
		this, SLOT(onSelectionChanged(const QItemSelection&, const QItemSelection&)));
	connect(ui_->fileTree, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(onTreeCustomContextMenu()));
}

void Project::onCreateFolder() {
	Debug::Log("onCreateFolder");
}

void Project::onCreateEmptyShader() {
	Debug::Log("onCreateEmptyShader");
}

void Project::onCreateImageEffectShader() {
	Debug::Log("onCreateImageEffectShader");
}

void Project::onOpenSelected() {
	QModelIndexList indexes = ui_->fileTree->selectionModel()->selectedIndexes();
	for (QModelIndex index : indexes) {
		QFileInfo info(model_->filePath(index));
		if (!info.isFile()) { continue; }

		QDesktopServices::openUrl(model_->filePath(index));

		// SUEDE TODO: multi-selection.
		break;
	}
}

void Project::onDeleteSelected() {
	QModelIndexList indexes = ui_->fileTree->selectionModel()->selectedIndexes();
	QString message("Are you sure to delete selected resource?");
	QDir dir("resources");
	for (QModelIndex index : indexes) {
		message += "\n" + dir.relativeFilePath(model_->filePath(index));
		// SUEDE TODO: multi-selection.
		break;
	}

	if (QMessageBox::warning(this,
		"Delete Selected Resource", message, QMessageBox::Yes, QMessageBox::No) == QMessageBox::Yes) {
		Debug::Log("Delete resource here");
	}
}

void Project::onShowSelectedInExplorer() {
	QStringList args({ "/select,", "" });
	QModelIndexList indexes = ui_->fileTree->selectionModel()->selectedIndexes();

	QProcess* process = new QProcess(this);
	for (QModelIndex index : indexes) {
		args.back() = QDir::toNativeSeparators(model_->filePath(index));
		process->start("explorer.exe", args);
		// SUEDE TODO: multi-selection.
		break;
	}
}

void Project::onTreeCustomContextMenu() {
	QModelIndexList indexes = ui_->fileTree->selectionModel()->selectedIndexes();
	if (indexes.empty()) { return; }

	QMenu menu;

	QMenu* create = menu.addMenu("Create");
	QAction* createFolder = new QAction("Folder", create);
	connect(createFolder, SIGNAL(triggered()), this, SLOT(onCreateFolder()));
	create->addAction(createFolder);

	create->addSeparator();

	QMenu* createShader = create->addMenu("Shader");
	QAction* emptyShader = new QAction("Empty Shader", createShader);
	connect(emptyShader, SIGNAL(triggered()), this, SLOT(onCreateEmptyShader()));

	QAction* imageEffectShader = new QAction("Image Effect Shader", createShader);
	connect(imageEffectShader, SIGNAL(triggered()), this, SLOT(onCreateImageEffectShader()));

	createShader->addAction(emptyShader);
	createShader->addAction(imageEffectShader);

	QAction* showInExplorer = new QAction("Show In Explorer", &menu);
	connect(showInExplorer, SIGNAL(triggered()), this, SLOT(onShowSelectedInExplorer()));

	QAction* open = new QAction("Open", &menu);
	connect(open, SIGNAL(triggered()), this, SLOT(onOpenSelected()));

	QAction* del = new QAction("Delete", &menu);
	connect(del, SIGNAL(triggered()), this, SLOT(onDeleteSelected()));

	menu.addAction(showInExplorer);
	menu.addAction(open);
	menu.addAction(del);
	menu.exec(QCursor::pos());
}

void Project::onSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected) {

}
