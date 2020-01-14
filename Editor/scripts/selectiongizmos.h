#pragma once
#include <QObject>

#include "gameobject.h"
#include "gizmospainter.h"

class Gizmos;
class Camera;
class Selection;
class SelectionGizmos : public QObject, public GizmosPainter {
	Q_OBJECT
	SUEDE_DECLARE_COMPONENT()

public:
	~SelectionGizmos() {}

public:
	void setSelection(Selection* value) { selection_ = value; }

public:
	virtual void Awake();
	virtual void OnDrawGizmos();

private:
	Gizmos* gizmos_;
	Camera* camera_;

	Selection* selection_;
};
