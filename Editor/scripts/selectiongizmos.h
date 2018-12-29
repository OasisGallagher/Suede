#pragma once
#include <QObject>

#include "gameobject.h"
#include "gizmospainter.h"

class SelectionGizmos : public QObject, public IGizmosPainter {
	Q_OBJECT
	SUEDE_DECLARE_COMPONENT()
public:
	~SelectionGizmos() {}

public:
	void setSelection(const QList<GameObject>& value);

public:
	virtual void OnDrawGizmos();

private:
	QList<IGameObject*> selection_;
};
