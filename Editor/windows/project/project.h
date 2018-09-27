#pragma once

#include <QDockWidget>
#include <QItemSelection>

#include "../winbase.h"

class QSignalMapper;
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
	QString newFolderName(const QString& parent);
	QStringList selectedEntries(QWidget* sender);

	void openEntries(const QStringList& entries);
	void removeEntries(const QStringList& selected);
	void showContents(const QStringList& directories);
};
