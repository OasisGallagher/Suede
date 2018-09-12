#pragma once
#include <vector>
#include <glm/glm.hpp>

#include "polygon.h"

enum class PlaneSide {
	Coinciding,
	Behind,
	Infront,
	Spanning,
};

class Plane;
class GeometryUtility {
public:
	static bool AARectContains(const glm::vec3& position, const glm::vec3& tl, const glm::vec3& rb);
	static bool PolygonContains(const glm::vec3* vertices, uint nvertices, const glm::vec3& position, const glm::vec3& normal, bool onEdge = true);

	/**
	 * @warning triangulate 2D polyon only.
	 */
	static void Triangulate(std::vector<glm::vec3>& triangles, const std::vector<glm::vec3>& polygon, const glm::vec3& normal);
	static void ClampTriangle(std::vector<glm::vec3>& polygon, const Triangle& triangle, const Plane* planes, uint count);

	static bool IsFrontFace(const Triangle& face, const glm::vec3& camera);
	static PlaneSide TestSide(const Plane& plane, const glm::vec3* points, uint npoints);

	static void GetSphereCoodrinates(std::vector<glm::vec3>& points, std::vector<uint>& indexes, const glm::ivec2& resolution);
	static void GetCuboidCoordinates(std::vector<glm::vec3>& points, const glm::vec3& center, const glm::vec3& size, std::vector<uint>* triangles = nullptr);

	/**
	 * @returns true if one or more points are inside the plane array.
	 */
	static bool PlanesCulling(Plane* planes, uint nplanes, const glm::vec3* points, uint npoints);

	static float GetDistance(const Plane& plane, const glm::vec3& p);
	static bool GetIntersection(glm::vec3& intersection, const Plane& plane, const glm::vec3& p0, const glm::vec3& p1);
	static void CalculateFrustumPlanes(Plane(&planes)[6], const glm::mat4& worldToClipMatrix);

private:
	GeometryUtility();
};
