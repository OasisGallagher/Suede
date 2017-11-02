#include <QTreeView>
#include <QHeaderView>
#include <QAbstractItemView>
#include <QStandardItemModel>

#include "sprite.h"
#include "engine.h"
#include "hierarchy.h"

Hierarchy* Hierarchy::get() {
	static Hierarchy instance;
	return &instance;
}

Hierarchy::Hierarchy() : model_(nullptr) {
}

void Hierarchy::initialize() {
	if (model_ != nullptr) { delete model_; }
	model_ = new QStandardItemModel(view_);
	
	tree_ = view_->findChild<QTreeView*>("tree", Qt::FindDirectChildrenOnly);
	tree_->setModel(model_);
	tree_->setHeaderHidden(true);

	view_->setSize(80, 200);
}

void Hierarchy::update(Sprite root) {
	model_->setRowCount(0);
	
	updateRecursively(root, nullptr);

	tree_->header()->resizeSections(QHeaderView::ResizeToContents);
}

Sprite Hierarchy::selectedSprite() {
	QModelIndex index = tree_->selectionModel()->currentIndex();
	if (!index.isValid()) { return nullptr; }

	unsigned id = model_->itemFromIndex(index)->data().toUInt();
	return Engine::get()->world()->GetSprite(id);
}

bool Hierarchy::selectedSprites(QList<Sprite>& sprites) {
	QModelIndexList indexes = tree_->selectionModel()->selectedIndexes();
	foreach(QModelIndex index, indexes) {
		unsigned id = model_->itemFromIndex(index)->data().toUInt();
		sprites.push_back(Engine::get()->world()->GetSprite(id));
	}

	return !sprites.empty();
}

void Hierarchy::updateRecursively(Sprite pp, QStandardItem* pi) {
	for (int i = 0; i < pp->GetChildCount(); ++i) {
		Sprite child = pp->GetChildAt(i);

		QStandardItem* item = new QStandardItem(child->GetName().c_str());
		item->setData(child->GetInstanceID());

		if (pi != nullptr) {
			pi->appendRow(item);
		}
		else {
			model_->appendRow(item);
		}
		
		updateRecursively(child, item);
	}
}
