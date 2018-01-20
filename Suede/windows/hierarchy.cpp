#include <QTreeView>
#include <QHeaderView>
#include <QAbstractItemView>
#include <QStandardItemModel>

#include "world.h"
#include "engine.h"
#include "hierarchy.h"

Hierarchy* hierarchyInstance;

Hierarchy* Hierarchy::get() {
	return hierarchyInstance;
}

Hierarchy::Hierarchy(QWidget* parent) : model_(nullptr), QDockWidget(parent) {
	hierarchyInstance = this;
}

Hierarchy::~Hierarchy() {
	hierarchyInstance = nullptr;
}

void Hierarchy::ready() {
	WorldInstance()->AddEventListener(this);

	model_ = new QStandardItemModel(this);
	
	tree_ = findChild<QTreeView*>("tree", Qt::FindChildrenRecursively);
	tree_->setModel(model_);
	tree_->setHeaderHidden(true);

	connect(tree_->selectionModel(), SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)), 
		this, SLOT(onSelectionChanged(const QItemSelection&, const QItemSelection&)));
}

Sprite Hierarchy::selectedSprite() {
	QModelIndex index = tree_->selectionModel()->currentIndex();
	if (!index.isValid()) { return nullptr; }

	uint id = model_->itemFromIndex(index)->data().toUInt();
	return WorldInstance()->GetSprite(id);
}

bool Hierarchy::selectedSprites(QList<Sprite>& sprites) {
	QModelIndexList indexes = tree_->selectionModel()->selectedIndexes();

	foreach (QModelIndex index, indexes) {
		uint id = model_->itemFromIndex(index)->data().toUInt();
		sprites.push_back(WorldInstance()->GetSprite(id));
	}

	return !sprites.empty();
}

void Hierarchy::OnWorldEvent(const WorldEventBase* e) {
	switch (e->GetEventType()) {
	case WorldEventTypeSpriteCreated:
		model_->setRowCount(0);
		updateRecursively(WorldInstance()->GetRootSprite(), nullptr);
		tree_->header()->resizeSections(QHeaderView::ResizeToContents);
		break;
	}
}

void Hierarchy::onSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected) {
	enableItemsOutline(selected, true);
	enableItemsOutline(deselected, false);
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

void Hierarchy::enableSpriteOutline(Sprite sprite, bool enable) {
	if (!sprite || !sprite->GetRenderer()) {
		return;
	}

	for (int i = 0; i < sprite->GetRenderer()->GetMaterialCount(); ++i) {
		Material material = sprite->GetRenderer()->GetMaterial(i);
		int outline = material->GetPassIndex("Outline");
		if (outline < 0) { continue; }

		if (enable) {
			material->EnablePass(outline);
		}
		else {
			material->DisablePass(outline);
		}
	}
}

void Hierarchy::enableItemsOutline(const QItemSelection& items, bool enable) {
	foreach(QItemSelectionRange r, items) {
		foreach(QModelIndex index, r.indexes()) {
			uint id = model_->itemFromIndex(index)->data().toUInt();
			Sprite sprite = WorldInstance()->GetSprite(id);
			enableSpriteOutline(sprite, enable);
		}
	}
}
