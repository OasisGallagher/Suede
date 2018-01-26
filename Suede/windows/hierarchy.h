#pragma once
#include <QMap>
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
	virtual void init(Ui::Suede* ui);

public:
	Sprite selectedSprite();
	bool selectedSprites(QList<Sprite>& sprites);
	void updateRecursively(Sprite pp, QStandardItem* pi);

signals:
	void selectionChanged(const QList<Sprite>& selected, const QList<Sprite>& deselected);

private:
	virtual void OnWorldEvent(const WorldEventBase* e);

private slots:
	void reload();
	void onSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected);

private:
	void appendChildItem(Sprite sprite);
	QStandardItem* appendItem(Sprite child, QStandardItem* pi);
	void removeItem(QStandardItem* item);

	void onSpriteTagChanged(Sprite sprite);

	void onSpriteNameChanged(Sprite sprite);
	void onSpriteParentChanged(Sprite sprite);

	void enableSpriteOutline(Sprite sprite, bool enable);
	void enableItemsOutline(const QList<Sprite>& sprites, bool enable);

	void selectionToSprites(QList<Sprite>& sprites, const QItemSelection& items);

private:
	QStandardItemModel* model_;

	typedef QMap<uint, QStandardItem*> ItemContainer;
	ItemContainer items_;
};
