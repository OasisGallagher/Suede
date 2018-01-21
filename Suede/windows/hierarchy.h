#pragma once
#include <QDockWidget>
#include <QItemSelection>

#include "childwindow.h"

#include "world.h"
#include "sprite.h"

class QTreeView;
class QStandardItem;
class QStandardItemModel;

class Hierarchy : public QDockWidget, public ChildWindow, public WorldEventListener {
	Q_OBJECT

public:
	static Hierarchy* get();

public:
	Hierarchy(QWidget* parent);
	~Hierarchy();

public:
	virtual void init();

public:
	Sprite selectedSprite();
	bool selectedSprites(QList<Sprite>& sprites);
	void updateRecursively(Sprite pp, QStandardItem* pi);

private:
	virtual void OnWorldEvent(const WorldEventBase* e);

private slots:
	void onSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected);

private:
	void enableSpriteOutline(Sprite sprite, bool enable);
	void enableItemsOutline(const QItemSelection& items, bool enable);

private:
	QTreeView* tree_;
	QStandardItemModel* model_;
};
