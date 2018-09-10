#include <list>

#include "plane.h"
#include "tools/math2.h"
#include "geometryutility.h"
#include "containers/arraylist.h"

#define IsZero(f)	(fabs((f)) < 1e-5)

struct EarVertex {
	enum { Reflex = 1, Ear = 2, };

	EarVertex() : mask(0), earListIndex(-1) {}
	EarVertex(const glm::vec3& value) : EarVertex() { position = value; }

	bool SetMask(int value, bool addMask);
	bool TestMask(int value) const { return (mask & value) != 0; }

	int mask;
	int earListIndex;
	glm::vec3 position;
};

static void ClampPolygon(std::list<glm::vec3>& list, const Plane& plane);
static void RemovePointsBehindPlane(std::list<glm::vec3>& list, const Plane& plane);

static uint CountPointsNotBehindPlanes(const Plane* planes, uint nplanes, const glm::vec3* points, uint npoints);
static bool GetUniqueIntersection(glm::vec3& intersection, const Plane& plane, const glm::vec3& prev, const glm::vec3& next);

static bool IsEar(array_list<EarVertex>& vertices, int current, const glm::vec3& normal);
static bool IsReflex(array_list<EarVertex>& vertices, int index, const glm::vec3& normal);
static int UpdateEarVertexState(array_list<EarVertex>& vertices, int vertexIndex, const glm::vec3& normal);

static void EarClipping(std::vector<glm::vec3>& triangles, array_list<EarVertex>& vertices, array_list<int>& earTips, const glm::vec3& normal);
static void EarClippingTriangulate(std::vector<glm::vec3>& triangles, const std::vector<glm::vec3>& polygon, const glm::vec3& normal);

bool GeometryUtility::AARectContains(const glm::vec3& point, const glm::vec3& tl, const glm::vec3& rb) {
	float xMin = tl.x, xMax = rb.x;
	if (xMin > xMax) { float tmp = xMin; xMin = xMax; xMax = tmp; }

	float zMin = tl.z, zMax = rb.z;
	if (zMin > zMax) { float tmp = zMin; zMin = zMax; zMax = tmp; }

	return point.x >= xMin && point.x <= xMax && point.z >= zMin && point.z <= zMax;
}

bool GeometryUtility::PolygonContains(const glm::vec3* vertices, uint nvertices, const glm::vec3& point, const glm::vec3& normal, bool onEdge) {
	for (uint i = 1; i <= nvertices; ++i) {
		const glm::vec3& currentPosition = i < nvertices ? vertices[i] : vertices[0];
		float cr = Math::Angle(glm::normalize(currentPosition - vertices[i - 1]), glm::normalize(point - vertices[i - 1]), normal);
		if (IsZero(cr) && AARectContains(point, currentPosition, vertices[i - 1])) {
			return onEdge;
		}

		if (cr > 0) {
			return false;
		}
	}

	return true;
}

void GeometryUtility::Triangulate(std::vector<glm::vec3>& triangles, const std::vector<glm::vec3>& polygon, const glm::vec3& normal) {
	if (polygon.size() == 3) {
		triangles.insert(triangles.end(), polygon.begin(), polygon.end());
	}
	else if (polygon.size() > 3) {
		EarClippingTriangulate(triangles, polygon, normal);
	}
}

void GeometryUtility::ClampTriangle(std::vector<glm::vec3>& polygon, const Triangle& triangle, const Plane* planes, uint count) {
	if (CountPointsNotBehindPlanes(planes, count, triangle.points, 3) == 3) {
		polygon.insert(polygon.end(), triangle.points, triangle.points + 3);
	}
	else {
		std::list<glm::vec3> list(triangle.points, triangle.points + 3);

		for (int pi = 0; list.size() >= 3 && pi < count; ++pi) {
			ClampPolygon(list, planes[pi]);
		}

		polygon.insert(polygon.end(), list.begin(), list.end());
	}
}

bool GeometryUtility::IsFrontFace(const Triangle& face, const glm::vec3& camera) {
	glm::vec3 normal = glm::cross(face[1] - face[0], face[2] - face[1]);
	return glm::dot(normal, face[1] - camera) < 0;
}

float GeometryUtility::GetDistance(const Plane& plane, const glm::vec3& p) {
	return glm::dot(plane.GetNormal(), p) + plane.GetDistance();
}

bool GeometryUtility::GetIntersection(glm::vec3& intersection, const Plane& plane, const glm::vec3& p0, const glm::vec3& p1) {
	float d0 = GetDistance(plane, p0);
	float d1 = GetDistance(plane, p1);

	if (IsZero(d0 * d1) || d0 * d1 >= 0) {
		return false;
	}

	float t = d0 / (d0 - d1);
	intersection = p0 + t * (p1 - p0);
	return true;
}

void GeometryUtility::CalculateFrustumPlanes(Plane(&planes)[6], const glm::mat4& worldToClipMatrix) {
#define EXTRACT_PLANE(index, sign, row)	\
	planes[index] = Plane(glm::vec4(worldToClipMatrix[0][3] sign worldToClipMatrix[0][row], \
		worldToClipMatrix[1][3] sign worldToClipMatrix[1][row], \
		worldToClipMatrix[2][3] sign worldToClipMatrix[2][row], \
		worldToClipMatrix[3][3] sign worldToClipMatrix[3][row]))

	EXTRACT_PLANE(0, +, 0);
	EXTRACT_PLANE(1, -, 0);
	EXTRACT_PLANE(2, +, 1);
	EXTRACT_PLANE(3, -, 1);
	EXTRACT_PLANE(4, +, 2);
	EXTRACT_PLANE(5, -, 2);

#undef EXTRACT_PLANE
}

//void GeometryUtility::CalculateFrustumPlanes(Plane(&planes)[6], Camera camera) {
//	CalculateFrustumPlanes(planes, camera->GetProjectionMatrix() * camera->GetTransform()->GetWorldToLocalMatrix());
/*
	vec3 nearCenter = camPos - camForward * nearDistance;
	vec3 farCenter = camPos - camForward * farDistance;

	float nearHeight = 2 * tanf(fovRadians/ 2) * nearDistance;
	float farHeight = 2 * tanf(fovRadians / 2) * farDistance;
	float nearWidth = nearHeight * viewRatio;
	float farWidth = farHeight * viewRatio;

	vec3 farTopLeft = farCenter + camUp * (farHeight*0.5) - camRight * (farWidth*0.5);
	vec3 farTopRight = farCenter + camUp * (farHeight*0.5) + camRight * (farWidth*0.5);
	vec3 farBottomLeft = farCenter - camUp * (farHeight*0.5) - camRight * (farWidth*0.5);
	vec3 farBottomRight = farCenter - camUp * (farHeight*0.5) + camRight * (farWidth*0.5);

	vec3 nearTopLeft = nearCenter + camY * (nearHeight*0.5) - camX * (nearWidth*0.5);
	vec3 nearTopRight = nearCenter + camY * (nearHeight*0.5) + camX * (nearWidth*0.5);
	vec3 nearBottomLeft = nearCenter - camY * (nearHeight*0.5) - camX * (nearWidth*0.5);
	vec3 nearBottomRight = nearCenter - camY * (nearHeight*0.5) + camX * (nearWidth*0.5);
	vec3 p0, p1, p2;

	p0 = nearBottomLeft; p1 = farBottomLeft; p2 = farTopLeft;
	vec3 leftPlaneNormal = Normalize(Cross(p1-p0, p2-p1));
	vec3 leftPlaneOffset = Dot(leftPlaneNormal, p0);

	p0 = nearTopLeft; p1 = farTopLeft; p2 = farTopRight;
	vec3 topPlaneNormal = Normalize(Cross(p1-p0, p2-p1));
	vec3 topPlaneNormal = Dot(topPlaneNormal , p0);

	p0 = nearTopRight; p1 = farTopRight; p2 = farBottomRight;
	vec3 rightPlaneNormal = Normalize(Cross(p1-p0, p2-p1));
	vec3 rightPlaneNormal = Dot(rightPlaneNormal , p0);

	p0 = nearBottomRight; p1 = farBottomRight; p2 = farBottomLeft;
	vec3 bottomPlaneNormal = Normalize(Cross(p1-p0, p2-p1));
	vec3 bottomPlaneNormal = Dot(bottomPlaneNormal , p0);
	*/
//}

PlaneSide GeometryUtility::TestSide(const Plane& plane, const glm::vec3* points, uint npoints) {
	uint npositive = 0, nnegative = 0;
	for (uint j = 0; j < npoints; ++j) {
		float f = GeometryUtility::GetDistance(plane, points[j]);
		if (f < 0) { ++nnegative; }
		else if (f > 0) { ++npositive; }
	}

	if (npositive == 0 && nnegative == 0) { return PlaneSide::Coinciding; }

	if (npositive == 0) { return PlaneSide::Behind; }

	if (nnegative == 0) { return PlaneSide::Infront; }

	return PlaneSide::Spanning;
}

void GeometryUtility::GetCuboidCoordinates(std::vector<glm::vec3>& points, const glm::vec3& center, const glm::vec3& size, std::vector<uint>* triangles) {
	glm::vec3 half = size / 2.f;

	points.assign({
		center + glm::vec3(half.xy, -half.z),
		center + glm::vec3(-half.x, half.y, -half.z),
		center + glm::vec3(-half.x, half.yz),
		center + half,
		center + glm::vec3(half.x, glm::vec2(0) - half.yz),
		center + (-half),
		center + glm::vec3(glm::vec2(0) - half.xy, half.z),
		center + glm::vec3(half.x, -half.y, half.z),
	});

	if (triangles != nullptr) {
		triangles->assign({
			0, 1, 2, 0, 2, 3, 0, 4, 5, 0, 5, 1,
			1, 5, 6, 1, 6, 2, 2, 6, 7, 2, 7, 3,
			3, 7, 4, 3, 4, 0, 4, 7, 6, 4, 6, 5
		});
	}
}

bool GeometryUtility::PlanesCulling(Plane* planes, uint nplanes, const glm::vec3* points, uint npoints) {
	for (uint i = 0; i < npoints; ++i) {
		bool inside = true;
		for (uint j = 0; j < nplanes; ++j) {
			if (TestSide(planes[j], points + i, 1) != PlaneSide::Infront) {
				inside = false;
			}
		}

		if (inside) { return true; }
	}

	return false;
}

uint CountPointsNotBehindPlanes(const Plane* planes, uint nplanes, const glm::vec3* points, uint npoints) {
	uint count = 0;
	for (uint i = 0; i < npoints; ++i) {
		bool inside = true;
		for (uint j = 0; j < nplanes; ++j) {
			if (GeometryUtility::TestSide(planes[j], points + i, 1) == PlaneSide::Behind) {
				inside = false;
			}
		}

		if (inside) { ++count; }
	}

	return count;
}

bool GetUniqueIntersection(glm::vec3& intersection, const Plane& plane, const glm::vec3& prev, const glm::vec3& next) {
	if (!GeometryUtility::GetIntersection(intersection, plane, prev, next)) {
		return false;
	}

	if (prev == intersection || next == intersection) {
		Debug::LogWarning("line start or end");
		return false;
	}

	return true;
}

void RemovePointsBehindPlane(std::list<glm::vec3>& list, const Plane& plane) {
	for (std::list<glm::vec3>::iterator current = list.begin(); current != list.end();) {
		float f = GeometryUtility::GetDistance(plane, *current);
		if (!IsZero(f) && f < 0) {
			current = list.erase(current);
		}
		else {
			++current;
		}
	}
}

void EarClippingTriangulate(std::vector<glm::vec3>& triangles, const std::vector<glm::vec3>& polygon, const glm::vec3& normal) {
	array_list<EarVertex> vertices(polygon.size());
	for (int i = 0; i < polygon.size(); ++i) {
		vertices.add(EarVertex(polygon[i]));
	}

	array_list<int> earTips(polygon.size());

	for (int index = 0; index < polygon.size(); ++index) {
		if (IsReflex(vertices, index, normal)) {
			vertices[index].SetMask(EarVertex::Reflex, true);
		}
		else if (IsEar(vertices, index, normal)) {
			vertices[index].SetMask(EarVertex::Ear, true);
			vertices[index].earListIndex = earTips.add(index);
		}
	}

	EarClipping(triangles, vertices, earTips, normal);
}

void ClampPolygon(std::list<glm::vec3>& list, const Plane& plane) {
	glm::vec3 intersection;
	std::list<glm::vec3>::iterator prev = list.begin(), next = prev;

	for (++next; next != list.end(); ) {
		if (GetUniqueIntersection(intersection, plane, *prev, *next)) {
			list.insert(next, intersection);
		}

		prev = next++;
	}

	if (GetUniqueIntersection(intersection, plane, list.back(), list.front())) {
		list.push_back(intersection);
	}

	RemovePointsBehindPlane(list, plane);
}

bool IsEar(array_list<EarVertex>& vertices, int current, const glm::vec3& normal) {
	if (vertices.size() < 3) { return false; }

	int prev = vertices.prev_index(current);
	int next = vertices.next_index(current);

	glm::vec3 points[] = {
		vertices[prev].position,
		vertices[current].position,
		vertices[next].position,
	};

	glm::vec3 c = glm::cross(points[0] - points[1], points[2] - points[1]);
	// Collinear.
	if (IsZero(glm::dot(c, c))) {
		return false;
	}

	for (array_list<EarVertex>::iterator ite = vertices.begin(); ite != vertices.end(); ++ite) {
		if (ite->index() == current || ite->index() == prev || ite->index() == next) {
			continue;
		}

		if (GeometryUtility::PolygonContains(points, CountOf(points), vertices[ite->index()].position, normal)) {
			return false;
		}
	}

	return true;
}

bool IsReflex(array_list<EarVertex>& vertices, int index, const glm::vec3& normal) {
	glm::vec3 current = vertices[index].position;
	glm::vec3 prev = vertices.prev_value(index).position;
	glm::vec3 next = vertices.next_value(index).position;
	return Math::Angle(glm::normalize(next - current), glm::normalize(prev - current), normal) < 0;
}

void EarClipping(std::vector<glm::vec3>& triangles, array_list<EarVertex>& vertices, array_list<int>& earTips, const glm::vec3& normal) {
	int earTipIndex = -1;
	for (array_list<int>::iterator ite = earTips.begin(); ite != earTips.end(); ++ite) {
		if (earTipIndex >= 0) { earTips.erase(earTipIndex); }

		earTipIndex = ite->index();

		int earTipVertexIndex = earTips[earTipIndex];
		EarVertex& earTipVertex = vertices[earTipVertexIndex];

		int prevVertexIndex = vertices.prev_index(earTipVertexIndex);
		EarVertex& prevVertex = vertices.prev_value(earTipVertexIndex);

		int nextVertexIndex = vertices.next_index(earTipVertexIndex);
		EarVertex& nextVertex = vertices.next_value(earTipVertexIndex);

		triangles.push_back(prevVertex.position);
		triangles.push_back(earTipVertex.position);
		triangles.push_back(nextVertex.position);

		vertices.erase(earTipVertexIndex);

		int state = UpdateEarVertexState(vertices, prevVertexIndex, normal);
		if (state > 0) {
			prevVertex.earListIndex = earTips.add(prevVertexIndex);
		}
		else if (state < 0) {
			earTips.erase(prevVertex.earListIndex);
		}

		state = UpdateEarVertexState(vertices, nextVertexIndex, normal);
		if (state > 0) {
			nextVertex.earListIndex = earTips.add(nextVertexIndex);
		}
		else if (state < 0) {
			earTips.erase(nextVertex.earListIndex);
		}
	}

	if (earTipIndex >= 0) { earTips.erase(earTipIndex); }
}

int UpdateEarVertexState(array_list<EarVertex>& vertices, int vertexIndex, const glm::vec3& normal) {
	EarVertex& earVertex = vertices[vertexIndex];

	int result = 0;

	bool isEar = earVertex.TestMask(EarVertex::Ear);

	if (earVertex.TestMask(EarVertex::Reflex)) {
		if (isEar) {
			Debug::Break();
		}

		if (!earVertex.SetMask(EarVertex::Reflex, IsReflex(vertices, vertexIndex, normal))
			&& earVertex.SetMask(EarVertex::Ear, IsEar(vertices, vertexIndex, normal))) {
			result = 1;
		}
	}
	else if (isEar != earVertex.SetMask(EarVertex::Ear, IsEar(vertices, vertexIndex, normal))) {
		result = 1;
		if (isEar) { result = -result; }
	}

	return result;
}

bool EarVertex::SetMask(int value, bool addMask) {
	if (addMask) { mask |= value; }
	else { mask &= (~value); }
	return addMask;
}
