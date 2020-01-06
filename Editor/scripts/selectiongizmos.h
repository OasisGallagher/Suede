#pragma once
#include <QObject>

#include "gameobject.h"
#include "gizmospainter.h"

class Gizmos;
class SelectionGizmos : public QObject, public GizmosPainter {
	Q_OBJECT
	SUEDE_DECLARE_COMPONENT()

public:
	~SelectionGizmos() {}

public:
	void setSelection(const QList<GameObject*>& value);

public:
	virtual void Awake();
	virtual void OnDrawGizmos();

private:
	Gizmos* gizmos_;
	QList<GameObject*> selection_;
};
