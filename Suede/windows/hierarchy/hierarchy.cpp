#include <QTreeView>
#include <QMimedata>
#include <QKeyEvent>

#include "ui_suede.h"

#include "hierarchy.h"
#include "os/filesystem.h"
#include "dragdropableitemmodel.h"

Hierarchy::Hierarchy(QWidget* parent) : model_(nullptr), QDockWidget(parent) {
}

void Hierarchy::init(Ui::Suede* ui) {
	WinBase::init(ui);

	World::get()->AddEventListener(this);

	setAcceptDrops(true);

	model_ = new DragDropableItemModel(this);
	
	ui_->tree->setModel(model_);
	ui_->tree->setHeaderHidden(true);
	ui_->tree->setDragDropMode(QAbstractItemView::DragDrop);

	connect(ui_->tree->selectionModel(), SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)), 
		this, SLOT(onSelectionChanged(const QItemSelection&, const QItemSelection&)));
	connect(ui_->tree, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(onTreeCustomContextMenu()));

	connect(ui->tree, SIGNAL(doubleClicked(const QModelIndex&)), this, SLOT(onEntityDoubleClicked(const QModelIndex&)));
}

void Hierarchy::OnEntityImported(Entity root, const std::string& path) {
	root->GetTransform()->SetParent(World::get()->GetRootTransform());
}

Entity Hierarchy::selectedEntity() {
	QModelIndex index = ui_->tree->selectionModel()->currentIndex();
	if (!index.isValid()) { return nullptr; }

	uint id = model_->itemFromIndex(index)->data().toUInt();
	return World::get()->GetEntity(id);
}

bool Hierarchy::selectedEntities(QList<Entity>& entities) {
	QModelIndexList indexes = ui_->tree->selectionModel()->selectedIndexes();

	foreach (QModelIndex index, indexes) {
		uint id = model_->itemFromIndex(index)->data().toUInt();
		entities.push_back(World::get()->GetEntity(id));
	}

	return !entities.empty();
}

void Hierarchy::OnWorldEvent(WorldEventBasePointer entit) {
	EntityEventPointer eep = suede_static_cast<EntityEventPointer>(entit);
	switch (entit->GetEventType()) {
//		case WorldEventTypeEntityCreated:
//			onEntityCreated(eep->entity);
//			break;
		case WorldEventTypeEntityDestroyed:
			onEntityDestroyed(eep->entity);
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

void Hierarchy::onEntityDestroyed(Entity entity) {
	QStandardItem* item = items_.value(entity->GetInstanceID());
	if (item != nullptr) {
		removeItem(item);
	}

	bool contains = false;
	for (QModelIndex index : ui_->tree->selectionModel()->selectedIndexes()) {
		uint id = model_->itemFromIndex(index)->data().toUInt();
		if (id == entity->GetInstanceID()) {
			contains = true;
			break;
		}
	}

	if (contains) {
		emit selectionChanged(QList<Entity>(), QList<Entity>({ entity }));
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

	updateRecursively(World::get()->GetRootTransform()->GetEntity(), nullptr);
}

void Hierarchy::onEntityDoubleClicked(const QModelIndex& index) {
	uint id = model_->itemFromIndex(index)->data().toUInt();
	Entity entity = World::get()->GetEntity(id);

	emit focusEntity(entity);
}

void Hierarchy::onTreeCustomContextMenu() {
	QMenu menu;
	QAction* del = new QAction(tr("Delete"), &menu);
	connect(del, SIGNAL(triggered()), this, SLOT(onDeleteSelected()));

	QModelIndexList indexes = ui_->tree->selectionModel()->selectedIndexes();
	if (indexes.empty()) {
		del->setEnabled(false);
	}

	menu.addAction(del);
	menu.exec(QCursor::pos());
}

void Hierarchy::onDeleteSelected() {
	QModelIndexList indexes = ui_->tree->selectionModel()->selectedIndexes();
	foreach(QModelIndex index, indexes) {
		World::get()->DestroyEntity(model_->itemFromIndex(index)->data().toUInt());
	}
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

void Hierarchy::updateRecursively(Entity entity, QStandardItem* parent) {
	for (int i = 0; i < entity->GetTransform()->GetChildCount(); ++i) {
		Entity child = entity->GetTransform()->GetChildAt(i)->GetEntity();
		QStandardItem* item = appendItem(child, parent);
		updateRecursively(child, item);
	}
}

void Hierarchy::keyReleaseEvent(QKeyEvent* event) {
	switch (event->key()) {
		case Qt::Key_Delete:
			onDeleteSelected();
			break;
	}
}

void Hierarchy::dropEvent(QDropEvent* event) {
	QDockWidget::dropEvent(event);
	QList<QUrl> urls = event->mimeData()->urls();
	foreach(QUrl url, urls) {
		std::string path = FileSystem::GetFileName(url.toString().toStdString());
		World::get()->Import(path, this);
	}
}

void Hierarchy::dragEnterEvent(QDragEnterEvent* event) {
	QDockWidget::dragEnterEvent(event);
	if (dropAcceptable(event->mimeData())) {
		event->acceptProposedAction();
	}
}

QStandardItem* Hierarchy::appendItem(Entity entity, QStandardItem* parent) {
	QStandardItem* item = items_.value(entity->GetInstanceID());
	if (item != nullptr) {
		removeItem(item);
		//QStandardItem* p = item->parent();
		//items_.remove(entity->GetInstanceID());
		//model_->removeRow(item->row(), p != nullptr ? p->index() : QModelIndex());
	}

	item = new QStandardItem(entity->GetName().c_str());
	item->setData(entity->GetInstanceID());
	
	if (parent != nullptr) {
		parent->appendRow(item);
	}
	else {
		model_->appendRow(item);
	}

	items_[entity->GetInstanceID()] = item;
	return item;
}

void Hierarchy::removeItem(QStandardItem* item) {
	removeItemRecusively(item);

	QModelIndex p;
	if (item->parent() != nullptr) { p = item->parent()->index(); }
	model_->removeRow(item->row(), p);
}

bool Hierarchy::dropAcceptable(const QMimeData* data) {
	if(data->hasFormat("targets")) {
		return true;
	}

	if (!data->hasUrls()) { return false; }

	foreach(QUrl url, data->urls()) {
		if (!url.toString().endsWith(".fbx") && !url.toString().endsWith(".obj")) {
			return false;
		}
	}

	return true;
}

void Hierarchy::appendChildItem(Entity entity) {
	Transform parent = entity->GetTransform()->GetParent();
	if (!parent) { return; }

	QStandardItem* item = nullptr;

	// append child node.
	if (parent == World::get()->GetRootTransform() || (item = items_.value(parent->GetEntity()->GetInstanceID())) != nullptr) {
		QStandardItem* pi = appendItem(entity, item);
		updateRecursively(entity, pi);
	}
	// remove orphan node.
	else if (item == nullptr && (item = items_.value(entity->GetInstanceID())) != nullptr) {
		removeItem(item);
	}
	//else {
	// parent node does not exist. 
	//	Debug::Break();
	//}
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
	foreach(QModelIndex index, items.indexes()) {
		uint id = model_->itemFromIndex(index)->data().toUInt();
		Entity entity = World::get()->GetEntity(id);
		if (entity) {
			entities.push_back(entity);
		}
	}
}

void Hierarchy::enableItemsOutline(const QList<Entity>& entities, bool enable) {
	foreach (Entity entity, entities) {
		enableEntityOutline(entity, enable);
	}
}

void Hierarchy::removeItemRecusively(QStandardItem* item) {
	for (int i = 0; i < item->rowCount(); ++i) {
		removeItemRecusively(item->child(i));
	}

	items_.remove(item->data().toUInt());
}
