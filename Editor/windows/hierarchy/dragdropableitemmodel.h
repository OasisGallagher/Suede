#pragma once
#include <QMimeData>
#include <QStandardItemModel>

#include "world.h"
#include "gameobject.h"

class DragDropableItemModel : public QStandardItemModel {
	Q_OBJECT

public:
	DragDropableItemModel(QObject* parent = Q_NULLPTR);

public:
	virtual Qt::ItemFlags flags(const QModelIndex& index) const;
	virtual QMimeData* mimeData(const QModelIndexList& indexes) const;
	virtual bool dropMimeData(const QMimeData* data, Qt::DropAction action, int row, int column, const QModelIndex& parent);
	virtual QStringList mimeTypes() const { return QStringList() << "drag"; }
};
