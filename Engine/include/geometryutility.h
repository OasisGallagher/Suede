#pragma once
#include <vector>

#include "plane.h"
#include "polygon.h"

#include "math/vector2.h"
#include "math/vector3.h"
#include "math/matrix4.h"

enum class PlaneSide {
	Coinciding,
	Behind,
	Infront,
	Spanning,
};

class GeometryUtility {
public:
	static bool AARectContains(const Vector3& position, const Vector3& tl, const Vector3& rb);
	static bool PolygonContains(const Vector3* vertices, uint nvertices, const Vector3& position, const Vector3& normal, bool onEdge = true);

	/**
	 * @warning triangulate 2D polyon only.
	 */
	static void Triangulate(std::vector<Vector3>& triangles, const std::vector<Vector3>& polygon, const Vector3& normal);
	static void ClampTriangle(std::vector<Vector3>& polygon, const Triangle& triangle, const Plane* planes, uint count);

	static bool IsFrontFace(const Triangle& face, const Vector3& camera);
	static PlaneSide TestSide(const Plane& plane, const Vector3* points, uint npoints);

	static void GetSphereCoodrinates(std::vector<Vector3>& points, std::vector<uint>& indexes, const Vector2& resolution);
	static void GetCuboidCoordinates(std::vector<Vector3>& points, const Vector3& center, const Vector3& size, std::vector<uint>* triangles = nullptr);

	/**
	 * @returns true if one or more points are inside the plane array.
	 */
	static bool PlanesCulling(Plane* planes, uint nplanes, const Vector3* points, uint npoints);

	static bool GetIntersection(Vector3& intersection, const Plane& plane, const Vector3& p0, const Vector3& p1);
	static void CalculateFrustumPlanes(Plane(&planes)[6], const Matrix4& worldToClipMatrix);

private:
	GeometryUtility();
};
