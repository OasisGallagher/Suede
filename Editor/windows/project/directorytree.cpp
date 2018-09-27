#include "directorytree.h"

#include <QDir>
#include <QHeaderView>
#include <QFileIconProvider>

#include "debug/debug.h"

DirectoryTree::DirectoryTree(QWidget* parent) : QTreeView(parent), watcher_(this) {
	model_ = new QStandardItemModel(this);
	setModel(model_);

	setHeaderHidden(true);
	setContextMenuPolicy(Qt::CustomContextMenu);

	connect(selectionModel(), SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)),
		this, SLOT(onSelectionChanged(const QItemSelection&, const QItemSelection&)));

	connect(this, SIGNAL(expanded(const QModelIndex&)), this, SLOT(onExpanded(const QModelIndex&)));
	connect(this, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(onCustomContextMenu()));

	connect(&watcher_, SIGNAL(directoryChanged(const QString&)), this, SLOT(onDirectoryChanged(const QString&)));
}

DirectoryTree::~DirectoryTree() {
}

void DirectoryTree::setRootPath(const QString& path) {
	setupFileSystemWatcher(path);

	QFileInfo fileInfo(path);
	if (!fileInfo.isDir()) {
		Debug::LogError("path %s must be a directory.");
		return;
	}
	
	model_->setRowCount(0);

	QFileIconProvider ip;
	QStandardItem* root = new QStandardItem(ip.icon(fileInfo), fileInfo.fileName());
	root->setEditable(false);
	model_->appendRow(root);

	// mark item as "unitialized" by adding "*" at front.
	root->setData("*" + path);
	initializeChildren(root);
}

void DirectoryTree::selectDirectory(const QString& path) {
	QList<QStandardItem*> list = findItem(path);
	for (QStandardItem* item : list) {
		expand(item->index());
	}

	if (!list.empty()) {
		selectionModel()->select(list.back()->index(), QItemSelectionModel::ClearAndSelect);
	}
}

void DirectoryTree::setupFileSystemWatcher(const QString& path) {
	QStringList list = watcher_.directories();
	if (!list.empty()) {
		watcher_.removePaths(list);
	}

	watcher_.addPath(path);
}

void DirectoryTree::onDirectoryChanged(const QString& path) {
	QList<QStandardItem*> list = findItem(path);

	if (list.empty()) {
		watcher_.removePath(path);
		return;
	}

	QString str = list.back()->data().toString();
	if (!str.startsWith("*")) {
		list.back()->setData("*" + str);
		initializeChildren(list.back());

		if (directories_.contains(str)) {
			emit selectionChanged(directories_);
		}
	}
}

QList<QStandardItem*> DirectoryTree::findItem(const QString& path) {
	QList<QStandardItem*> list;
	if (!QFileInfo(path).exists()) {
		return list;
	}

	QStandardItem* item = nullptr;
	for (const QString& p : path.split("/")) {
		if (item == nullptr) {
			list.push_back(item = model_->item(0));
			continue;
		}

		int n = list.size();
		for (int i = 0; i < item->rowCount(); ++i) {
			if (item->child(i)->text() == p) {
				list.push_back(item = item->child(i));
				break;
			}
		}

		// item at path does not exist.
		if (list.size() == n) {
			list.clear();
			break;
		}
	}

	return list;
}

void DirectoryTree::onCustomContextMenu() {
	QModelIndexList indexes = selectionModel()->selectedIndexes();
	if (!indexes.empty()) {
		emit requestContextMenuOnItems(directories_);
	}
}

void DirectoryTree::onSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected) {
	for (const QModelIndex& index : selected.indexes()) {
		QStandardItem* item = model_->itemFromIndex(index);
		QString path = item->data().toString();
		initializeChildren(item);

		if (!directories_.contains(path)) {
			directories_.push_back(path);
		}
	}

	for (const QModelIndex& index : deselected.indexes()) {
		QString path = model_->itemFromIndex(index)->data().toString();
		directories_.removeAll(path);
	}

	emit selectionChanged(directories_);
}

void DirectoryTree::onExpanded(const QModelIndex& index) {
	QStandardItem* item = model_->itemFromIndex(index);
	for (int i = 0; i < item->rowCount(); ++i) {
		QStandardItem* child = item->child(i);
		initializeChildren(child);
	}
}

void DirectoryTree::initializeChildren(QStandardItem* item) {
	QString path = item->data().toString();

	// already initialzed.
	if (!path.startsWith("*")) {
		return;
	}
	
	item->removeRows(0, item->rowCount());

	path = path.right(path.length() - 1);

	QDir dir(path);
	QFileIconProvider ip;
	for (const QFileInfo& info : dir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot)) {
		QStandardItem* child = new QStandardItem(ip.icon(info), info.fileName());
		child->setEditable(false);

		item->appendRow(child);
		child->setData("*" + info.filePath());
		watcher_.addPath(info.filePath());
	}

	item->setData(path);
}
