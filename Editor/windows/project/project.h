#pragma once

#include <QDir>
#include <QDockWidget>
#include <QItemSelection>
#include <QAbstractItemDelegate>

#include "../winbase.h"

class QListWidgetItem;
class QFileSystemModel;
class Project : public QDockWidget, public WinSingleton<Project> {
	Q_OBJECT

public:
	Project(QWidget* parent);

public:
	virtual void init(Ui::Editor* ui);
	virtual void awake() {}

private slots:
	void onAddressChanged();

	void onItemEdited(QWidget* widget, QAbstractItemDelegate::EndEditHint hint);

	void onItemClicked(QListWidgetItem* item);
	void onItemDoubleClicked(QListWidgetItem* item);

	void onItemChanged(QListWidgetItem* item);
	void onCreateFolder(const QStringList& selected);

	void onCreateEmptyShader(const QStringList& selected);
	void onCreateImageEffectShader(const QStringList& selected);

	void onOpenSelected(const QStringList& selected);
	void onDeleteSelected(const QStringList& selected);

	void onShowSelectedInExplorer(const QStringList& selected);

	void onCustomContextMenu();
	void onSelectionChanged(const QStringList& directories);

private:
	QString folderPath(const QString& path);
	QSet<QString> entriesInFolder(const QString& folder, QDir::Filters filter);

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
	bool tryRenameEntry(QListWidgetItem* item);

	void openEntries(const QStringList& entries);
	void removeEntries(const QStringList& selected);
	void showDirectortiesContent(const QStringList& directories);

private:
	QSet<QString> builtinEntries_;
};
