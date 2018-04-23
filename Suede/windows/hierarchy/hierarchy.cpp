#include <QTreeView>
#include <QHeaderView>
#include <QAbstractItemView>
#include <QStandardItemModel>

#include "ui_suede.h"

#include "world.h"
#include "engine.h"
#include "hierarchy.h"
#include "debug/debug.h"

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

	connect(ui->tree, SIGNAL(doubleClicked(const QModelIndex&)), this, SLOT(onEntityDoubleClicked(const QModelIndex&)));
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

void Hierarchy::OnWorldEvent(WorldEventBasePointer e) {
	EntityEventPointer eep = suede_static_cast<EntityEventPointer>(e);
	switch (e->GetEventType()) {
		case WorldEventTypeEntityCreated:
			onEntityCreated(eep->entity);
			break;
		case WorldEventTypeEntityTagChanged:
			onEntityTagChanged(eep->entity);
			break;
		case WorldEventTypeEntityNameChanged:
			onEntityNameChanged(eep->entity);
			break;
		case WorldEventTypeEntityParentChanged:
			onEntityParentChanged(eep->entity);
			break;
		case WorldEventTypeEntityActiveChanged:
			onEntityActiveChanged(eep->entity);
			break;
	}
}

void Hierarchy::onEntityCreated(Entity entity) {
	QStandardItem* item = new QStandardItem(entity->GetName().c_str());
	item->setData(entity->GetInstanceID());
	model_->appendRow(item);
	items_[entity->GetInstanceID()] = item;
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

void Hierarchy::onEntityActiveChanged(Entity entity) {
	QStandardItem* item = items_.value(entity->GetInstanceID());
	if (item != nullptr) {
		static QBrush activeNameBrush(QColor(0xFFFFFF));
		static QBrush inactiveNameBrush(QColor(0x666666));
		item->setForeground(entity->GetActive() ? activeNameBrush : inactiveNameBrush);
	}
}

void Hierarchy::reload() {
	items_.clear();
	model_->setRowCount(0);

	updateRecursively(WorldInstance()->GetRootTransform()->GetEntity(), nullptr);
}

void Hierarchy::onEntityDoubleClicked(const QModelIndex& index) {
	uint id = model_->itemFromIndex(index)->data().toUInt();
	Entity entity = WorldInstance()->GetEntity(id);

	emit focusEntity(entity);
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
		QStandardItem* item = appendItem(child, pi);
		updateRecursively(child, item);
	}
}

QStandardItem* Hierarchy::appendItem(Entity entity, QStandardItem* pi) {
	QStandardItem* item = items_.value(entity->GetInstanceID());
	if (item != nullptr) {
		QStandardItem* p = item->parent();
		items_.remove(entity->GetInstanceID());
		model_->removeRow(item->row(), p != nullptr ? p->index() : QModelIndex());
	}

	item = new QStandardItem(entity->GetName().c_str());
	item->setData(entity->GetInstanceID());
	
	if (pi != nullptr) {
		pi->appendRow(item);
	}
	else {
		model_->appendRow(item);
	}

	items_[entity->GetInstanceID()] = item;
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
	if (parent == WorldInstance()->GetRootTransform() || (pi = items_.value(parent->GetEntity()->GetInstanceID())) != nullptr) {
		appendItem(entity, pi);
	}
	else if (pi == nullptr && (pi = items_.value(entity->GetInstanceID())) != nullptr) {
		removeItem(pi);
	}
	else {
		Debug::Break();
	}
}

void Hierarchy::enableEntityOutline(Entity entity, bool enable) {
	Renderer renderer;
	if (!entity || !(renderer = entity->GetRenderer())) {
		return;
	}

	for (int i = 0; i < renderer->GetMaterialCount(); ++i) {
		Material material = renderer->GetMaterial(i);
		int outline = material->FindPass("Outline");
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
