#pragma once

#include <QDockWidget>
#include <QItemSelection>

#include "../winbase.h"

class QFileSystemModel;
class Project : public QDockWidget, public WinSingleton<Project> {
	Q_OBJECT

public:
	Project(QWidget* parent);

public:
	virtual void init(Ui::Editor* ui);
	virtual void awake() {}

private slots:
	void onCreateFolder();
	void onCreateEmptyShader();
	void onCreateImageEffectShader();

	void onOpenSelected();
	void onDeleteSelected();
	void onShowSelectedInExplorer();
	void onTreeCustomContextMenu();
	void onSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected);

private:
	QFileSystemModel* model_;
};
