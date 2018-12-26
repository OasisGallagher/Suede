#pragma once
#include <QObject>

#include "gizmos.h"
#include "gameobject.h"
#include "gizmospainter.h"

class SelectionGizmos : public QObject, public GizmosPainter {
	Q_OBJECT
	SUEDE_DECLARE_COMPONENT()
public:
	~SelectionGizmos() {}

public:
	void setSelection(const QList<GameObject>& value);

public:
	virtual void OnDrawGizmos();

private:
	QList<suede_weak_ref<GameObject>> selection_;
};
