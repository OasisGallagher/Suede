#include "dragdropableitemmodel.h"

DragDropableItemModel::DragDropableItemModel(QObject* parent) : QStandardItemModel(parent) {

}

Qt::ItemFlags DragDropableItemModel::flags(const QModelIndex& index) const {
	return QStandardItemModel::flags(index) | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled;
}

QMimeData* DragDropableItemModel::mimeData(const QModelIndexList& indexes) const {
	if (indexes.count() == 0) { return nullptr; }

	QMimeData* data = new QMimeData;
	QString targets;
	foreach(QModelIndex index, indexes) {
		if (!targets.isEmpty()) { targets += "|"; }
		targets += QString::number(itemFromIndex(index)->data().toUInt());
	}

	data->setData("drag", "dragItem");
	data->setData("targets", targets.toLocal8Bit());
	return data;
}

bool DragDropableItemModel::dropMimeData(const QMimeData* data, Qt::DropAction action, int row, int column, const QModelIndex& parent) {
	QStandardItem* item = itemFromIndex(parent);
	Entity parentEntity = World::get()->GetRootTransform()->GetEntity();
	if (item != nullptr) {
		parentEntity = World::get()->GetEntity(item->data().toUInt());
	}

	foreach(QString target, QString(data->data("targets")).split('|')) {
		Entity entity = World::get()->GetEntity(target.toUInt());
		entity->GetTransform()->SetParent(parentEntity->GetTransform());
	}

	return true;
}
