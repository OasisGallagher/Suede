#pragma once
#include <QItemSelection>

#include "world.h"
#include "sprite.h"
#include "window.h"

class QTreeView;
class QStandardItem;
class QStandardItemModel;

class Hierarchy : public Window, public WorldEventListener {
	Q_OBJECT

public:
	static Hierarchy* get();

public:
	Sprite selectedSprite();
	bool selectedSprites(QList<Sprite>& sprites);

private:
	Hierarchy();
	~Hierarchy();

	virtual void initialize();
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
