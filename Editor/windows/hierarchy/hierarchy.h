#pragma once
#include <QMap>
#include <QDockWidget>
#include <QItemSelection>

#include "../winbase.h"

#include "world.h"
#include "gameobject.h"

class QTreeView;
class QStandardItem;
class QStandardItemModel;

class Hierarchy : public QDockWidget, public WinSingleton<Hierarchy>, public WorldEventListener, public GameObjectLoadedListener {
	Q_OBJECT

public:
	Hierarchy(QWidget* parent);

public:
	virtual void init(Ui::Editor* ui);
	virtual void awake();

public:
	virtual void OnGameObjectImported(GameObject root, const std::string& path);

public:
	GameObject selectedGameObject();
	bool selectedGameObjects(QList<GameObject>& gameObjects);
	void updateRecursively(GameObject go, QStandardItem* parent);

protected:
	virtual void dropEvent(QDropEvent* event);
	virtual void keyReleaseEvent(QKeyEvent* event);
	virtual void dragEnterEvent(QDragEnterEvent* event);

signals:
	void focusGameObject(GameObject go);
	void selectionChanged(const QList<GameObject>& selected, const QList<GameObject>& deselected);
	
private:
	virtual void OnWorldEvent(WorldEventBasePointer e);

private slots:
	void reload();
	void onDeleteSelected();
	void onTreeCustomContextMenu();
	void onGameObjectDoubleClicked(const QModelIndex&);
	void onSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected);

private:
	void appendChildItem(GameObject go);
	QStandardItem* appendItem(GameObject child, QStandardItem* parent);
	void removeItem(QStandardItem* item);
	void removeItemRecusively(QStandardItem* item);

	bool dropAcceptable(const QMimeData* data);

	void onGameObjectCreated(GameObject go);
	void onGameObjectDestroyed(GameObject go);
	void onGameObjectTagChanged(GameObject go);
	void onGameObjectNameChanged(GameObject go);
	void onGameObjectParentChanged(GameObject go);
	void onGameObjectActiveChanged(GameObject go);

	void enableGameObjectOutline(GameObject go, bool enable);
	void enableItemsOutline(const QList<GameObject>& gameObjects, bool enable);
	void selectionToGameObjects(QList<GameObject>& gameObjects, const QItemSelection& items);

private:
	QStandardItemModel* model_;

	typedef QMap<uint, QStandardItem*> ItemContainer;
	ItemContainer items_;
};

