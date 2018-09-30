#pragma once
#include <QObject>

#include "gizmos.h"
#include "gameobject.h"
#include "gizmospainter.h"

class SelectionGizmos : public QObject, public GizmosPainter {
	Q_OBJECT
	SUEDE_DECLARE_COMPONENT()

public:
	void setSelection(const QList<GameObject>& value) { selection_ = value; }

public:
	virtual void OnDrawGizmos();

private:
	QList<GameObject> selection_;
};
