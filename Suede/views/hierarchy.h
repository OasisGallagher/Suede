#pragma once
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

	virtual void initialize();
	void updateRecursively(Sprite pp, QStandardItem* pi);

private:
	QTreeView* tree_;
	QStandardItemModel* model_;
};
