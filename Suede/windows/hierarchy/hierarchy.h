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

class Hierarchy : public QDockWidget, public WinSingleton<Hierarchy>, public WorldEventListener {
	Q_OBJECT

public:
	Hierarchy(QWidget* parent);

public:
	virtual void init(Ui::Suede* ui);

public:
	Entity selectedEntity();
	bool selectedEntities(QList<Entity>& entities);
	void updateRecursively(Entity entity, QStandardItem* parent);

signals:
	void focusEntity(Entity entity);
	void selectionChanged(const QList<Entity>& selected, const QList<Entity>& deselected);
	
private:
	virtual void OnWorldEvent(WorldEventBasePointer e);

private slots:
	void reload();
	void onEntityDoubleClicked(const QModelIndex&);
	void onSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected);

private:
	void appendChildItem(Entity entity);
	QStandardItem* appendItem(Entity child, QStandardItem* parent);
	void removeItem(QStandardItem* item);

	void onEntityCreated(Entity entity);
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

