#pragma once
#include <QMap>
#include <QDockWidget>
#include <QItemSelection>

#include "../winbase.h"

#include "world.h"
#include "entity.h"

class QTreeView;
class QStandardItem;
class QStandardItemModel;

class Hierarchy : public QDockWidget, public WinSingleton<Hierarchy>, public WorldEventListener, public EntityLoadedListener {
	Q_OBJECT

public:
	Hierarchy(QWidget* parent);

public:
	virtual void init(Ui::Editor* ui);
	virtual void awake();

public:
	virtual void OnEntityImported(Entity root, const std::string& path);

public:
	Entity selectedEntity();
	bool selectedEntities(QList<Entity>& entities);
	void updateRecursively(Entity entity, QStandardItem* parent);

protected:
	virtual void dropEvent(QDropEvent* event);
	virtual void keyReleaseEvent(QKeyEvent* event);
	virtual void dragEnterEvent(QDragEnterEvent* event);

signals:
	void focusEntity(Entity entity);
	void selectionChanged(const QList<Entity>& selected, const QList<Entity>& deselected);
	
private:
	virtual void OnWorldEvent(WorldEventBasePointer e);

private slots:
	void reload();
	void onDeleteSelected();
	void onTreeCustomContextMenu();
	void onEntityDoubleClicked(const QModelIndex&);
	void onSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected);

private:
	void appendChildItem(Entity entity);
	QStandardItem* appendItem(Entity child, QStandardItem* parent);
	void removeItem(QStandardItem* item);
	void removeItemRecusively(QStandardItem* item);

	bool dropAcceptable(const QMimeData* data);

	void onEntityCreated(Entity entity);
	void onEntityDestroyed(Entity entity);
	void onEntityTagChanged(Entity entity);
	void onEntityNameChanged(Entity entity);
	void onEntityParentChanged(Entity entity);
	void onEntityActiveChanged(Entity entity);

	void enableEntityOutline(Entity entity, bool enable);
	void enableItemsOutline(const QList<Entity>& entities, bool enable);
	void selectionToEntities(QList<Entity>& entities, const QItemSelection& items);

private:
	QStandardItemModel* model_;

	typedef QMap<uint, QStandardItem*> ItemContainer;
	ItemContainer items_;
};

