#pragma once
#include <QMap>
#include <QDockWidget>
#include <QItemSelection>

#include "main/childwindow.h"

#include "world.h"
#include "gameobject.h"
#include "tools/event.h"

class QTreeView;
class QStandardItem;
class QStandardItemModel;

class HierarchyWindow : public ChildWindow, public WorldEventListener {
	Q_OBJECT

public:
	enum {
		WindowType = ChildWindowType::Hierarchy,
	};

public:
	HierarchyWindow(QWidget* parent);

public:
	virtual void initUI();
	virtual void awake();

public:
	GameObject* selectedGameObject();
	QList<GameObject*> selectedGameObjects();
	void setSelectedGameObjects(const QList<GameObject*>& objects);
	void updateRecursively(GameObject* go, QStandardItem* parent);

protected:
	virtual void dropEvent(QDropEvent* event);
	virtual void keyReleaseEvent(QKeyEvent* event);
	virtual void dragEnterEvent(QDragEnterEvent* event);

signals:
	void focusGameObject(GameObject* go);
	void selectionChanged(const QList<GameObject*>& selected, const QList<GameObject*>& deselected);

private slots:
	void reload();
	void onDeleteSelected();
	void onTreeCustomContextMenu();
	void onGameObjectDoubleClicked(const QModelIndex& index);
	void onSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected);

private:
	virtual void OnWorldEvent(WorldEventBasePtr e);

private:
	void appendChildItem(GameObject* go);
	QStandardItem* appendItem(GameObject* child, QStandardItem* parent);
	void removeItem(QStandardItem* item);
	void removeItemRecusively(QStandardItem* item);

	bool dropAcceptable(const QMimeData* data);

	void onGameObjectDestroyed(GameObject* go);
	void onGameObjectNameChanged(GameObject* go);
	void onGameObjectParentChanged(GameObject* go);
	void onGameObjectActiveChanged(GameObject* go);

	void enableGameObjectOutline(GameObject* go, bool enable);
	void enableGameObjectsOutline(const QList<GameObject*>& gameObjects, bool enable);
	void selectionToGameObjects(QList<GameObject*>& gameObjects, const QItemSelection& items);

	void onGameObjectImported(GameObject* root, const std::string& path);

private:
	QStandardItemModel* model_;

	typedef QMap<uint, QStandardItem*> ItemContainer;
	ItemContainer items_;
};
