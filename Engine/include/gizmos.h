#pragma once
#include "types.h"
#include "color.h"
#include "defines.h"
#include "subsystem.h"

class Graphics;
class SUEDE_API Gizmos : public Subsystem {
	SUEDE_DECLARE_IMPLEMENTATION(Gizmos)

public:
	enum {
		SystemType = SubsystemType::Gizmos,
	};

public:
	Gizmos(Graphics* graphics);

public:
	Matrix4 GetMatrix();
	void SetMatrix(const Matrix4& value);

	Color GetColor();
	void SetColor(const Color& value);

	void DrawLines(const Vector3* points, uint npoints);
	void DrawLines(const std::initializer_list<Vector3>& points);
	void DrawLines(const Vector3* points, uint npoints, const uint* indexes, uint nindexes);
	void DrawLines(const std::initializer_list<Vector3>& points, const std::initializer_list<uint>& indexes);

	void DrawLineStripe(const Vector3* points, uint npoints);
	void DrawLineStripe(const Vector3* points, uint npoints, const uint* indexes, uint nindexes);

	void DrawSphere(const Vector3& center, float radius);
	void DrawCuboid(const Vector3& center, const Vector3& size);

	// http://www.freemancw.com/2012/06/opengl-cone-function/
	void DrawCone() {}

	void DrawWireSphere(const Vector3& center, float radius);
	void DrawWireCuboid(const Vector3& center, const Vector3& size);

public:
	virtual void Awake();
	virtual void Update(float deltaTime);
};
