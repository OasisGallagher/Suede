#pragma once
#include "types.h"
#include "color.h"
#include "defines.h"
#include "tools/singleton.h"

class SUEDE_API Gizmos : private Singleton2<Gizmos> {
	friend class Singleton<Gizmos>;
	SUEDE_DECLARE_IMPLEMENTATION(Gizmos)

public:
	static void Flush();

	static Matrix4 GetMatrix();
	static void SetMatrix(const Matrix4& value);

	static Color GetColor();
	static void SetColor(const Color& value);

	static void DrawLines(const Vector3* points, uint npoints);
	static void DrawLines(const std::initializer_list<Vector3>& points);
	static void DrawLines(const Vector3* points, uint npoints, const uint* indexes, uint nindexes);
	static void DrawLines(const std::initializer_list<Vector3>& points, const std::initializer_list<uint>& indexes);

	static void DrawLineStripe(const Vector3* points, uint npoints);
	static void DrawLineStripe(const Vector3* points, uint npoints, const uint* indexes, uint nindexes);

	static void DrawSphere(const Vector3& center, float radius);
	static void DrawCuboid(const Vector3& center, const Vector3& size);

	// http://www.freemancw.com/2012/06/opengl-cone-function/
	static void DrawCone() {}

	static void DrawWireSphere(const Vector3& center, float radius);
	static void DrawWireCuboid(const Vector3& center, const Vector3& size);

private:
	Gizmos();
};
