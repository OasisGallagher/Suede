#include <QTreeView>
#include <QKeyEvent>

#include "ui_editor.h"

#include "hierarchy.h"
#include "os/filesystem.h"
#include "dragdropableitemmodel.h"

Hierarchy::Hierarchy(QWidget* parent) : model_(nullptr), QDockWidget(parent) {
}

void Hierarchy::init(Ui::Editor* ui) {
	WinBase::init(ui);

	setAcceptDrops(true);

	model_ = new DragDropableItemModel(this);
	
	ui_->gameObjectTree->setModel(model_);
	ui_->gameObjectTree->setHeaderHidden(true);
	ui_->gameObjectTree->setDragDropMode(QAbstractItemView::DragDrop);

	connect(ui_->gameObjectTree->selectionModel(), SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)),
		this, SLOT(onSelectionChanged(const QItemSelection&, const QItemSelection&)));
	connect(ui_->gameObjectTree, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(onTreeCustomContextMenu()));

	connect(ui->gameObjectTree, SIGNAL(doubleClicked(const QModelIndex&)), this, SLOT(onGameObjectDoubleClicked(const QModelIndex&)));
}

void Hierarchy::awake() {
	World::instance()->AddEventListener(this);
}

void Hierarchy::OnGameObjectImported(GameObject root, const std::string& path) {
	root->GetTransform()->SetParent(World::instance()->GetRootTransform());
}

GameObject Hierarchy::selectedGameObject() {
	QModelIndex index = ui_->gameObjectTree->selectionModel()->currentIndex();
	if (!index.isValid()) { return nullptr; }

	uint id = model_->itemFromIndex(index)->data().toUInt();
	return World::instance()->GetGameObject(id);
}

QList<GameObject> Hierarchy::selectedGameObjects() {
	QModelIndexList indexes = ui_->gameObjectTree->selectionModel()->selectedIndexes();

	QList<GameObject> gameObjects;
	gameObjects.reserve(indexes.size());

	for (QModelIndex index : indexes) {
		uint id = model_->itemFromIndex(index)->data().toUInt();
		gameObjects.push_back(World::instance()->GetGameObject(id));
	}

	return gameObjects;
}

void Hierarchy::OnWorldEvent(WorldEventBasePtr entit) {
	GameObjectEventPtr eep = suede_static_cast<GameObjectEventPtr>(entit);
	switch (entit->GetEventType()) {
//		case WorldEventType::GameObjectCreated:
//			onGameObjectCreated(eep->go);
//			break;
		case WorldEventType::GameObjectDestroyed:
			onGameObjectDestroyed(eep->go);
			break;
		case WorldEventType::GameObjectTagChanged:
			onGameObjectTagChanged(eep->go);
			break;
		case WorldEventType::GameObjectNameChanged:
			onGameObjectNameChanged(eep->go);
			break;
		case WorldEventType::GameObjectParentChanged:
			onGameObjectParentChanged(eep->go);
			break;
		case WorldEventType::GameObjectActiveChanged:
			onGameObjectActiveChanged(eep->go);
			break;
	}
}

void Hierarchy::onGameObjectCreated(GameObject go) {
	QStandardItem* item = new QStandardItem(go->GetName().c_str());
	item->setData(go->GetInstanceID());
	model_->appendRow(item);
	items_[go->GetInstanceID()] = item;
}

void Hierarchy::onGameObjectDestroyed(GameObject go) {
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
		emit selectionChanged(QList<GameObject>(), QList<GameObject>({ go }));
	}
}

void Hierarchy::onGameObjectTagChanged(GameObject go) {
}

void Hierarchy::onGameObjectNameChanged(GameObject go) {
	QStandardItem* item = items_.value(go->GetInstanceID());
	if (item != nullptr) {
		item->setText(go->GetName().c_str());
	}
}

void Hierarchy::onGameObjectParentChanged(GameObject go) {
	appendChildItem(go);
}

void Hierarchy::onGameObjectActiveChanged(GameObject go) {
	QStandardItem* item = items_.value(go->GetInstanceID());
	if (item != nullptr) {
		static QBrush activeNameBrush(QColor(0xFFFFFF));
		static QBrush inactiveNameBrush(QColor(0x666666));
		item->setForeground(go->GetActive() ? activeNameBrush : inactiveNameBrush);
	}
}

void Hierarchy::reload() {
	items_.clear();
	model_->setRowCount(0);

	updateRecursively(World::instance()->GetRootTransform()->GetGameObject(), nullptr);
}

void Hierarchy::onGameObjectDoubleClicked(const QModelIndex& index) {
	uint id = model_->itemFromIndex(index)->data().toUInt();
	GameObject go = World::instance()->GetGameObject(id);

	emit focusGameObject(go);
}

void Hierarchy::onTreeCustomContextMenu() {
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

void Hierarchy::onDeleteSelected() {
	QModelIndexList indexes = ui_->gameObjectTree->selectionModel()->selectedIndexes();
	for (QModelIndex index : indexes) {
		World::instance()->DestroyGameObject(model_->itemFromIndex(index)->data().toUInt());
	}
}

void Hierarchy::onSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected) {
	QList<GameObject> ss;
	selectionToGameObjects(ss, selected);
	enableGameObjectsOutline(ss, true);

	QList<GameObject> ds;
	selectionToGameObjects(ds, deselected);
	enableGameObjectsOutline(ds, false);

	emit selectionChanged(ss, ds);
}

void Hierarchy::updateRecursively(GameObject go, QStandardItem* parent) {
	for (Transform transform : go->GetTransform()->GetChildren()) {
		GameObject child = transform->GetGameObject();
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
	for (QUrl url : urls) {
		std::string path = FileSystem::GetFileName(url.toString().toStdString());
		World::instance()->Import(path, this);
	}
}

void Hierarchy::dragEnterEvent(QDragEnterEvent* event) {
	QDockWidget::dragEnterEvent(event);
	if (dropAcceptable(event->mimeData())) {
		event->acceptProposedAction();
	}
}

QStandardItem* Hierarchy::appendItem(GameObject go, QStandardItem* parent) {
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

	for (QUrl url : data->urls()) {
		if (!url.toString().endsWith(".fbx") && !url.toString().endsWith(".obj")) {
			return false;
		}
	}

	return true;
}

void Hierarchy::appendChildItem(GameObject go) {
	Transform parent = go->GetTransform()->GetParent();
	if (!parent) { return; }

	QStandardItem* item = nullptr;

	// append child node.
	if (parent == World::instance()->GetRootTransform() || (item = items_.value(parent->GetGameObject()->GetInstanceID())) != nullptr) {
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

void Hierarchy::enableGameObjectOutline(GameObject go, bool enable) {
	if (!go) { return; }

	MeshRenderer renderer = go->GetComponent<IMeshRenderer>();
	if (!renderer) { return; }

	for (Material material : renderer->GetMaterials()) {
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

void Hierarchy::selectionToGameObjects(QList<GameObject>& gameObjects, const QItemSelection& items) {
	for (QModelIndex index : items.indexes()) {
		uint id = model_->itemFromIndex(index)->data().toUInt();
		GameObject go = World::instance()->GetGameObject(id);
		if (go) {
			gameObjects.push_back(go);
		}
	}
}

void Hierarchy::enableGameObjectsOutline(const QList<GameObject>& gameObjects, bool enable) {
	for (GameObject go : gameObjects) {
		enableGameObjectOutline(go, enable);
	}
}

void Hierarchy::removeItemRecusively(QStandardItem* item) {
	for (int i = 0; i < item->rowCount(); ++i) {
		removeItemRecusively(item->child(i));
	}

	items_.remove(item->data().toUInt());
}
