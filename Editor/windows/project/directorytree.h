#pragma once
#include <QTreeView>
#include <QStandardItemModel>
#include <QFileSystemWatcher>

class DirectoryTree : public QTreeView {
	Q_OBJECT

public:
	DirectoryTree(QWidget* widget = Q_NULLPTR);
	~DirectoryTree();

public:
	void setRootPath(const QString& path);
	void selectDirectory(const QString& path);
	const QStringList& selectedDirectories() { return directories_; }

signals:
	void selectionChanged(const QStringList& directories);
	void requestContextMenuOnItems(const QStringList& directories);

protected:
	//virtual void selectionChanged(const QItemSelection& selected, const QItemSelection& deselected);
	 
private slots:
	void onCustomContextMenu();
	void onSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected);

	void onExpanded(const QModelIndex& index);
	void onDirectoryChanged(const QString& path);

private:
	void initializeChildren(QStandardItem* item);
	void setupFileSystemWatcher(const QString& path);

	QList<QStandardItem*> findItem(const QString& path);

private:
	QStringList directories_;
	QStandardItemModel* model_;
	QFileSystemWatcher watcher_;
};
