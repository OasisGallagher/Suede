#pragma once

#include <QDir>
#include <QDockWidget>
#include <QItemSelection>
#include <QAbstractItemDelegate>

#include "../winbase.h"
#include "os/filesystem.h"

class FileFinder;
class QListWidgetItem;
class QFileSystemModel;

class Project : public QDockWidget, public WinSingleton<Project> {
	Q_OBJECT

public:
	Project(QWidget* parent);
	~Project();

public:
	virtual void init(Ui::Editor* ui);
	virtual void awake() {}

private slots:
	void onAddressChanged();

	void onFindFileFieldChanged();

	void onItemEdited(QWidget* widget);
	void onItemChanged(QListWidgetItem* item);
	void onItemDoubleClicked(QListWidgetItem* item);

	void onCreateFolder(const QStringList& selected);
	void onCreateEmptyShader(const QStringList& selected);
	void onCreateImageEffectShader(const QStringList& selected);

	void onOpenSelected(const QStringList& selected);
	void onDeleteSelected(const QStringList& selected);

	void onShowSelectedInExplorer(const QStringList& selected);

	void onCustomContextMenu();
	void onDirectoryChanged(const QString& directory);
	void onSelectedDirectoriesChanged(const QStringList& directories);

private:
	QString folderPath(const QString& path);
	QSet<QString> entriesInFolder(const QString& folder, QDir::Filters filter);

	void reloadFindResults();
	void reloadSelectedDirectoriesContent();

	bool hasBuiltinEntry(const QStringList& selected);

	QString newFolderName(const QString& folder);
	QString newShaderName(const QString& folder);
	QString newEntryName(const QString& base, const QString& postfix, const QSet<QString>& used);

	QStringList selectedEntries(QWidget* sender);

	void createEmptyShader(const QString& path);
	void createImageEffectShader(const QString& path);

	void createFile(const QString &path, const QString& templatePath);
	void createEntryListItem(const QString& folder, const QString& name, const QString& magic, const QIcon& icon);

	bool createEntry(QListWidgetItem* item);
	bool renameEntry(QListWidgetItem* item);

	void openEntries(const QStringList& entries);
	void removeEntries(const QStringList& selected);

	void showFindResult(const QStringList& paths);
	void showDirectortiesContent(const QStringList& directories);

private:
	FileTree tree_;
	QSet<QString> builtinEntries_;
};
