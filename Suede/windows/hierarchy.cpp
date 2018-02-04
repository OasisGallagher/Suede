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

Entity Hierarchy::selectedEntity() {
	QModelIndex index = ui_->tree->selectionModel()->currentIndex();
	if (!index.isValid()) { return nullptr; }

	uint id = model_->itemFromIndex(index)->data().toUInt();
	return WorldInstance()->GetEntity(id);
}

bool Hierarchy::selectedEntities(QList<Entity>& entities) {
	QModelIndexList indexes = ui_->tree->selectionModel()->selectedIndexes();

	foreach (QModelIndex index, indexes) {
		uint id = model_->itemFromIndex(index)->data().toUInt();
		entities.push_back(WorldInstance()->GetEntity(id));
	}

	return !entities.empty();
}

void Hierarchy::OnWorldEvent(const WorldEventBase* e) {
	switch (e->GetEventType()) {
		case WorldEventTypeEntityTagChanged:
			onEntityTagChanged(((EntityEvent*)e)->entity);
			break;
		case WorldEventTypeEntityNameChanged:
			onEntityNameChanged(((EntityEvent*)e)->entity);
			break;
		case WorldEventTypeEntityParentChanged:
			onEntityParentChanged(((EntityEvent*)e)->entity);
			break;
	}
}

void Hierarchy::onEntityTagChanged(Entity entity) {
}

void Hierarchy::onEntityNameChanged(Entity entity) {
	QStandardItem* item = items_.value(entity->GetInstanceID());
	if (item != nullptr) {
		item->setText(entity->GetName().c_str());
	}
}

void Hierarchy::onEntityParentChanged(Entity entity) {
	appendChildItem(entity);
}

void Hierarchy::reload() {
	items_.clear();
	model_->setRowCount(0);

	updateRecursively(WorldInstance()->GetRootTransform()->GetEntity(), nullptr);
}

void Hierarchy::onSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected) {
	QList<Entity> ss;
	selectionToEntities(ss, selected);
	enableItemsOutline(ss, true);

	QList<Entity> ds;
	selectionToEntities(ds, deselected);
	enableItemsOutline(ds, false);

	emit selectionChanged(ss, ds);
}

void Hierarchy::updateRecursively(Entity entity, QStandardItem* pi) {
	for (int i = 0; i < entity->GetTransform()->GetChildCount(); ++i) {
		Entity child = entity->GetTransform()->GetChildAt(i)->GetEntity();
		if (child->GetType() == ObjectTypeSkybox) {
			continue;
		}

		QStandardItem* item = appendItem(child, pi);
		updateRecursively(child, item);
	}
}

QStandardItem* Hierarchy::appendItem(Entity transform, QStandardItem* pi) {
	QStandardItem* item = items_.value(transform->GetInstanceID());
	if (item != nullptr) {
		items_.remove(transform->GetInstanceID());
		model_->removeRow(item->row());
	}

	item = new QStandardItem(transform->GetName().c_str());
	item->setData(transform->GetInstanceID());
	
	if (pi != nullptr) {
		pi->appendRow(item);
	}
	else {
		model_->appendRow(item);
	}

	items_[transform->GetInstanceID()] = item;
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

void Hierarchy::appendChildItem(Entity entity) {
	Transform parent = entity->GetTransform()->GetParent();
	Q_ASSERT(parent);

	QStandardItem* pi = nullptr;
	if (parent == WorldInstance()->GetRootTransform() || (pi = items_.value(parent->GetInstanceID())) != nullptr) {
		appendItem(entity, pi);
	}
	else if (pi == nullptr && (pi = items_.value(entity->GetInstanceID())) != nullptr) {
		removeItem(pi);
	}
}

void Hierarchy::enableEntityOutline(Entity entity, bool enable) {
	if (!entity || !entity->GetRenderer()) {
		return;
	}

	for (int i = 0; i < entity->GetRenderer()->GetMaterialCount(); ++i) {
		Material material = entity->GetRenderer()->GetMaterial(i);
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

void Hierarchy::selectionToEntities(QList<Entity>& entities, const QItemSelection& items) {
	foreach(QItemSelectionRange r, items) {
		foreach(QModelIndex index, r.indexes()) {
			uint id = model_->itemFromIndex(index)->data().toUInt();
			Entity transform = WorldInstance()->GetEntity(id);
			Q_ASSERT(transform);
			entities.push_back(transform);
		}
	}
}

void Hierarchy::enableItemsOutline(const QList<Entity>& entities, bool enable) {
	foreach (Entity entity, entities) {
		enableEntityOutline(entity, enable);
	}
}
