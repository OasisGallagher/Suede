#include <QTreeView>
#include <QHeaderView>
#include <QAbstractItemView>
#include <QStandardItemModel>

#include "ui_suede.h"

#include "tags.h"
#include "world.h"
#include "engine.h"
#include "hierarchy.h"

static Hierarchy* hierarchyInstance;

Hierarchy* Hierarchy::get() {
	return hierarchyInstance;
}

Hierarchy::Hierarchy(QWidget* parent) : model_(nullptr), QDockWidget(parent) {
	hierarchyInstance = this;
}

Hierarchy::~Hierarchy() {
	hierarchyInstance = nullptr;
}

void Hierarchy::init(Ui::Suede* ui) {
	ChildWindow::init(ui);

	WorldInstance()->AddEventListener(this);

	model_ = new QStandardItemModel(this);
	
	ui_->tree->setModel(model_);
	ui_->tree->setHeaderHidden(true);

	connect(ui_->tree->selectionModel(), SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)), 
		this, SLOT(onSelectionChanged(const QItemSelection&, const QItemSelection&)));
}

Sprite Hierarchy::selectedSprite() {
	QModelIndex index = ui_->tree->selectionModel()->currentIndex();
	if (!index.isValid()) { return nullptr; }

	uint id = model_->itemFromIndex(index)->data().toUInt();
	return WorldInstance()->GetSprite(id);
}

bool Hierarchy::selectedSprites(QList<Sprite>& sprites) {
	QModelIndexList indexes = ui_->tree->selectionModel()->selectedIndexes();

	foreach (QModelIndex index, indexes) {
		uint id = model_->itemFromIndex(index)->data().toUInt();
		sprites.push_back(WorldInstance()->GetSprite(id));
	}

	return !sprites.empty();
}

void Hierarchy::OnWorldEvent(const WorldEventBase* e) {
	switch (e->GetEventType()) {
		case WorldEventTypeSpriteTagChanged:
			onSpriteTagChanged(((SpriteEvent*)e)->sprite);
			break;
		case WorldEventTypeSpriteNameChanged:
			onSpriteNameChanged(((SpriteEvent*)e)->sprite);
			break;
		case WorldEventTypeSpriteParentChanged:
			onSpriteParentChanged(((SpriteEvent*)e)->sprite);
			break;
	}
}

void Hierarchy::onSpriteTagChanged(Sprite sprite) {
}

void Hierarchy::onSpriteNameChanged(Sprite sprite) {
	QStandardItem* item = items_.value(sprite->GetInstanceID());
	if (item != nullptr) {
		item->setText(sprite->GetName().c_str());
	}
}

void Hierarchy::onSpriteParentChanged(Sprite sprite) {
	appendChildItem(sprite);
}

void Hierarchy::reload() {
	items_.clear();
	model_->setRowCount(0);

	updateRecursively(WorldInstance()->GetRootSprite(), nullptr);
}

void Hierarchy::onSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected) {
	QList<Sprite> ss;
	selectionToSprites(ss, selected);
	enableItemsOutline(ss, true);

	QList<Sprite> ds;
	selectionToSprites(ds, deselected);
	enableItemsOutline(ds, false);

	emit selectionChanged(ss, ds);
}

void Hierarchy::updateRecursively(Sprite pp, QStandardItem* pi) {
	for (int i = 0; i < pp->GetChildCount(); ++i) {
		Sprite child = pp->GetChildAt(i);
		if (child->GetType() == ObjectTypeSkybox) {
			continue;
		}

		QStandardItem* item = appendItem(child, pi);
		updateRecursively(child, item);
	}
}

QStandardItem* Hierarchy::appendItem(Sprite sprite, QStandardItem* pi) {
	QStandardItem* item = items_.value(sprite->GetInstanceID());
	if (item != nullptr) {
		items_.remove(sprite->GetInstanceID());
		model_->removeRow(item->row());
	}

	item = new QStandardItem(sprite->GetName().c_str());
	item->setData(sprite->GetInstanceID());
	
	if (pi != nullptr) {
		pi->appendRow(item);
	}
	else {
		model_->appendRow(item);
	}

	items_[sprite->GetInstanceID()] = item;
	return item;
}

void Hierarchy::removeItem(QStandardItem* item) {
	for (int i = 0; i < item->rowCount(); ++i) {
		foreach(QStandardItem* x, item->takeRow(i)) {
			items_.remove(x->data().toUInt());
			delete x;
		}
	}

	items_.remove(item->data().toUInt());
	model_->removeRow(item->row());
}

void Hierarchy::appendChildItem(Sprite sprite) {
	Sprite parent = sprite->GetParent();
	Q_ASSERT(parent);

	QStandardItem* pi = nullptr;
	if (parent == WorldInstance()->GetRootSprite() || (pi = items_.value(parent->GetInstanceID())) != nullptr) {
		appendItem(sprite, pi);
	}
	else if (pi == nullptr && (pi = items_.value(sprite->GetInstanceID())) != nullptr) {
		removeItem(pi);
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

void Hierarchy::selectionToSprites(QList<Sprite>& sprites, const QItemSelection& items) {
	foreach(QItemSelectionRange r, items) {
		foreach(QModelIndex index, r.indexes()) {
			uint id = model_->itemFromIndex(index)->data().toUInt();
			Sprite sprite = WorldInstance()->GetSprite(id);
			Q_ASSERT(sprite);
			sprites.push_back(sprite);
		}
	}
}

void Hierarchy::enableItemsOutline(const QList<Sprite>& sprites, bool enable) {
	foreach (Sprite sprite, sprites) {
		enableSpriteOutline(sprite, enable);
	}
}
