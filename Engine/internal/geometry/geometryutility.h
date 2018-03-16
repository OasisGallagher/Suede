#pragma once

#include <list>
#include <vector>
#include <glm/glm.hpp>

#include "triangle.h"
#include "containers/arraylist.h"

class Plane;
struct EarVertex;

class GeometryUtility {
public:
	static bool DiagonalRectContains(const glm::vec3& position, const glm::vec3& tl, const glm::vec3& rb);
	static bool PolygonContains(const glm::vec3* positions, uint npositions, const glm::vec3& position, const glm::vec3& normal, bool onEdge = true);

	static void Triangulate(std::vector<glm::vec3>& triangles, const std::vector<glm::vec3>& polygon, const glm::vec3& normal);
	static void ClampTriangle(std::vector<glm::vec3>& polygon, const Triangle& triangle, const Plane* planes, uint count);

	static bool IsFrontFace(const Triangle& triangle, const glm::vec3& camera);
	
	/**
	 * @return: 0: coinciding; 1: behind; 2: infront; 3: spanning 
	 */
	static int CalculateSide(const Plane* planes, uint nplanes, const glm::vec3* points, uint npoints);

	static float GetDistance(const Plane& plane, const glm::vec3& p);
	static bool GetIntersection(glm::vec3& intersection, const Plane& plane, const glm::vec3& p0, const glm::vec3& p1);
	static void CalculateFrustumPlanes(Plane(&planes)[6], const glm::mat4& worldToClipMatrix);

private:
	static void ClampPolygon(std::list<glm::vec3>& list, const Plane& plane);
	static void RemovePointsBehindPlane(std::list<glm::vec3>& list, const Plane& plane);

	static bool GetUniqueIntersection(glm::vec3& intersection, const Plane& plane, const glm::vec3& prev, const glm::vec3& next);

	static bool IsEar(array_list<EarVertex>& vertices, int current, const glm::vec3& normal);
	static bool IsReflex(array_list<EarVertex>& vertices, int index, const glm::vec3& normal);
	static int UpdateEarVertexState(array_list<EarVertex>& vertices, int vertexIndex, const glm::vec3& normal);

	static void EarClipping(std::vector<glm::vec3>& triangles, array_list<EarVertex>& vertices, array_list<int>& earTips, const glm::vec3& normal);
	static void EarClippingTriangulate(std::vector<glm::vec3>& triangles, const std::vector<glm::vec3>& polygon, const glm::vec3& normal);
};
