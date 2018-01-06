#pragma once
#include <QItemSelection>

#include "sprite.h"
#include "window.h"

class QTreeView;
class QStandardItem;
class QStandardItemModel;

class Hierarchy : public Window {
	Q_OBJECT

public:
	static Hierarchy* get();

public:
	void update(Sprite root);

	Sprite selectedSprite();
	bool selectedSprites(QList<Sprite>& sprites);

private:
	Hierarchy();
	~Hierarchy();

	virtual void initialize();
	void updateRecursively(Sprite pp, QStandardItem* pi);

private slots:
	void onSpriteClicked(const QItemSelection& selected, const QItemSelection& deselected);

	void EnableSpriteOutline(Sprite sprite, bool enable);
	void EnableItemsOutline(const QItemSelection& items, bool enable);

private:
	QTreeView* tree_;
	QStandardItemModel* model_;
};
