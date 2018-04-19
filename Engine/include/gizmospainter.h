#pragma once

class GizmosPainter {
public:
	virtual ~GizmosPainter() {}

public:
	virtual void OnDrawGizmos() = 0;
};
