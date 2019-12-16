#include <QTreeView>
#include <QKeyEvent>

#include "main/editor.h"

#include "hierarchywindow.h"
#include "os/filesystem.h"
#include "dragdropableitemmodel.h"

HierarchyWindow::HierarchyWindow(QWidget* parent) : ChildWindow(parent), model_(nullptr) {
	setAcceptDrops(true);

	model_ = new DragDropableItemModel(this);
}

void HierarchyWindow::initUI() {
	ui_->gameObjectTree->setModel(model_);
	ui_->gameObjectTree->setHeaderHidden(true);
	ui_->gameObjectTree->setDragDropMode(QAbstractItemView::DragDrop);

	connect(ui_->gameObjectTree->selectionModel(), SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)),
		this, SLOT(onSelectionChanged(const QItemSelection&, const QItemSelection&)));
	connect(ui_->gameObjectTree, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(onTreeCustomContextMenu()));

	connect(ui_->gameObjectTree, SIGNAL(doubleClicked(const QModelIndex&)), this, SLOT(onGameObjectDoubleClicked(const QModelIndex&)));
}

void HierarchyWindow::awake() {
	World::gameObjectImported.subscribe(this, &HierarchyWindow::onGameObjectImported);

	World::AddEventListener(this);
}

void HierarchyWindow::onGameObjectImported(GameObject* root, const std::string& path) {
	root->GetTransform()->SetParent(World::GetRootTransform());
}

GameObject* HierarchyWindow::selectedGameObject() {
	QModelIndex index = ui_->gameObjectTree->selectionModel()->currentIndex();
	if (!index.isValid()) { return nullptr; }

	uint id = model_->itemFromIndex(index)->data().toUInt();
	return World::GetGameObject(id);
}

QList<GameObject*> HierarchyWindow::selectedGameObjects() {
	QModelIndexList indexes = ui_->gameObjectTree->selectionModel()->selectedIndexes();

	QList<GameObject*> gameObjects;
	gameObjects.reserve(indexes.size());

	for (QModelIndex index : indexes) {
		uint id = model_->itemFromIndex(index)->data().toUInt();
		gameObjects.push_back(World::GetGameObject(id));
	}

	return gameObjects;
}

void HierarchyWindow::setSelectedGameObjects(const QList<GameObject*>& objects) {
	ui_->gameObjectTree->clearSelection();

	QStandardItem* item = nullptr;
	for (GameObject* go : objects) {
		if ((item = items_.value(go->GetInstanceID())) != nullptr) {
			ui_->gameObjectTree->selectionModel()->select(item->index(), QItemSelectionModel::Select);
		}
	}

	if (!objects.empty() && (item = items_.value(objects.front()->GetInstanceID())) != nullptr) {
		ui_->gameObjectTree->scrollTo(item->index());
	}
}

void HierarchyWindow::OnWorldEvent(WorldEventBasePtr entit) {
	GameObjectEventPtr eep = std::static_pointer_cast<GameObjectEvent>(entit);
	switch (entit->GetEventType()) {
		case WorldEventType::GameObjectDestroyed:
			onGameObjectDestroyed(eep->go.get());
			break;
		case WorldEventType::GameObjectNameChanged:
			onGameObjectNameChanged(eep->go.get());
			break;
		case WorldEventType::GameObjectParentChanged:
			onGameObjectParentChanged(eep->go.get());
			break;
		case WorldEventType::GameObjectActiveChanged:
			onGameObjectActiveChanged(eep->go.get());
			break;
	}
}

void HierarchyWindow::onGameObjectDestroyed(GameObject* go) {
	QStandardItem* item = items_.value(go->GetInstanceID());
	if (item != nullptr) {
		removeItem(item);
	}

	bool contains = false;
	for (QModelIndex index : ui_->gameObjectTree->selectionModel()->selectedIndexes()) {
		uint id = model_->itemFromIndex(index)->data().toUInt();
		if (id == go->GetInstanceID()) {
			contains = true;
			break;
		}
	}

	if (contains) {
		emit selectionChanged(QList<GameObject*>(), QList<GameObject*>({ go }));
	}
}

void HierarchyWindow::onGameObjectNameChanged(GameObject* go) {
	QStandardItem* item = items_.value(go->GetInstanceID());
	if (item != nullptr) {
		item->setText(go->GetName().c_str());
	}
}

void HierarchyWindow::onGameObjectParentChanged(GameObject* go) {
	appendChildItem(go);
}

void HierarchyWindow::onGameObjectActiveChanged(GameObject* go) {
	QStandardItem* item = items_.value(go->GetInstanceID());
	if (item != nullptr) {
		static QBrush activeNameBrush(QColor(0xFFFFFF));
		static QBrush inactiveNameBrush(QColor(0x666666));
		item->setForeground(go->GetActive() ? activeNameBrush : inactiveNameBrush);
	}
}

void HierarchyWindow::reload() {
	items_.clear();
	model_->setRowCount(0);

	updateRecursively(World::GetRootTransform()->GetGameObject(), nullptr);
}

void HierarchyWindow::onGameObjectDoubleClicked(const QModelIndex& index) {
	uint id = model_->itemFromIndex(index)->data().toUInt();
	GameObject* go = World::GetGameObject(id);

	emit focusGameObject(go);
}

void HierarchyWindow::onTreeCustomContextMenu() {
	QMenu menu;
	QAction* del = new QAction("Delete", &menu);
	connect(del, SIGNAL(triggered()), this, SLOT(onDeleteSelected()));

	QModelIndexList indexes = ui_->gameObjectTree->selectionModel()->selectedIndexes();
	if (indexes.empty()) {
		del->setEnabled(false);
	}

	menu.addAction(del);
	menu.exec(QCursor::pos());
}

void HierarchyWindow::onDeleteSelected() {
	QModelIndexList indexes = ui_->gameObjectTree->selectionModel()->selectedIndexes();
	for (QModelIndex index : indexes) {
		World::DestroyGameObject(model_->itemFromIndex(index)->data().toUInt());
	}
}

void HierarchyWindow::onSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected) {
	QList<GameObject*> ss;
	selectionToGameObjects(ss, selected);
	enableGameObjectsOutline(ss, true);

	QList<GameObject*> ds;
	selectionToGameObjects(ds, deselected);
	enableGameObjectsOutline(ds, false);

	emit selectionChanged(ss, ds);
}

void HierarchyWindow::updateRecursively(GameObject* go, QStandardItem* parent) {
	Transform* tr = go->GetTransform();
	for (int i = 0; i <tr->GetChildCount(); ++i) {
		GameObject* child = tr->GetChildAt(i)->GetGameObject();
		QStandardItem* item = appendItem(child, parent);
		updateRecursively(child, item);
	}
}

void HierarchyWindow::keyReleaseEvent(QKeyEvent* event) {
	switch (event->key()) {
		case Qt::Key_Delete:
			onDeleteSelected();
			break;
	}
}

void HierarchyWindow::dropEvent(QDropEvent* event) {
	QDockWidget::dropEvent(event);
	QList<QUrl> urls = event->mimeData()->urls();
	for (QUrl url : urls) {
		std::string path = FileSystem::GetFileName(url.toString().toStdString());
		World::Import(path);
	}
}

void HierarchyWindow::dragEnterEvent(QDragEnterEvent* event) {
	QDockWidget::dragEnterEvent(event);
	if (dropAcceptable(event->mimeData())) {
		event->acceptProposedAction();
	}
}

QStandardItem* HierarchyWindow::appendItem(GameObject* go, QStandardItem* parent) {
	QStandardItem* item = items_.value(go->GetInstanceID());
	if (item != nullptr) {
		removeItem(item);
		//QStandardItem* p = item->parent();
		//items_.remove(go->GetInstanceID());
		//model_->removeRow(item->row(), p != nullptr ? p->index() : QModelIndex());
	}

	item = new QStandardItem(go->GetName().c_str());
	item->setData(go->GetInstanceID());
	
	if (parent != nullptr) {
		parent->appendRow(item);
	}
	else {
		model_->appendRow(item);
	}

	items_[go->GetInstanceID()] = item;
	return item;
}

void HierarchyWindow::removeItem(QStandardItem* item) {
	removeItemRecusively(item);

	QModelIndex p;
	if (item->parent() != nullptr) { p = item->parent()->index(); }
	model_->removeRow(item->row(), p);
}

bool HierarchyWindow::dropAcceptable(const QMimeData* data) {
	if(data->hasFormat("targets")) {
		return true;
	}

	if (!data->hasUrls()) { return false; }

	for (QUrl url : data->urls()) {
		if (!url.toString().endsWith(".fbx") && !url.toString().endsWith(".obj")) {
			return false;
		}
	}

	return true;
}

void HierarchyWindow::appendChildItem(GameObject* go) {
	Transform* parent = go->GetTransform()->GetParent();
	if (parent == nullptr) { return; }

	QStandardItem* item = nullptr;

	// append child node.
	if (parent == World::GetRootTransform() || (item = items_.value(parent->GetGameObject()->GetInstanceID())) != nullptr) {
		QStandardItem* pi = appendItem(go, item);
		updateRecursively(go, pi);
	}
	// remove orphan node.
	else if (item == nullptr && (item = items_.value(go->GetInstanceID())) != nullptr) {
		removeItem(item);
	}
	//else {
	// parent node does not exist. 
	//	Debug::Break();
	//}
}

void HierarchyWindow::enableGameObjectOutline(GameObject* go, bool enable) {
	if (!go) { return; }

	MeshRenderer* renderer = go->GetComponent<MeshRenderer>();
	if (!renderer) { return; }

	for (int i = 0; i < renderer->GetMaterialCount(); ++i) {
		Material* material = renderer->GetMaterial(i);
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

void HierarchyWindow::selectionToGameObjects(QList<GameObject*>& gameObjects, const QItemSelection& items) {
	for (QModelIndex index : items.indexes()) {
		uint id = model_->itemFromIndex(index)->data().toUInt();
		GameObject* go = World::GetGameObject(id);
		if (go) {
			gameObjects.push_back(go);
		}
	}
}

void HierarchyWindow::enableGameObjectsOutline(const QList<GameObject*>& gameObjects, bool enable) {
	for (GameObject* go : gameObjects) {
		enableGameObjectOutline(go, enable);
	}
}

void HierarchyWindow::removeItemRecusively(QStandardItem* item) {
	for (int i = 0; i < item->rowCount(); ++i) {
		removeItemRecusively(item->child(i));
	}

	items_.remove(item->data().toUInt());
}
