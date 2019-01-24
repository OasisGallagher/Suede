#include <list>

#include "tools/math2.h"
#include "geometryutility.h"
#include "containers/arraylist.h"

#define ADD_TRIANGLE(c, i, j, k)	\
	c.push_back(i);	c.push_back(j); c.push_back(k)

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

static glm::vec3 SphereCoodrinate(float x, float y);

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
		float cr = glm::angle(glm::normalize(currentPosition - vertices[i - 1]), glm::normalize(point - vertices[i - 1]), normal);
		if (Math::Approximately(cr, 0) && AARectContains(point, currentPosition, vertices[i - 1])) {
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

	if (Math::Approximately(d0 * d1, 0) || d0 * d1 >= 0) {
		return false;
	}

	float t = d0 / (d0 - d1);
	intersection = p0 + t * (p1 - p0);
	return true;
}

void GeometryUtility::CalculateFrustumPlanes(Plane(&planes)[6], const glm::mat4& worldToClipMatrix) {
	CalculateFrustumPlanes((float*)planes, 0, 4, (float*)planes, 3, 4, (float*)&worldToClipMatrix);
}

void GeometryUtility::CalculateFrustumPlanes(float* normals, int normalOffset, int normalStride, float* distances, int distanceOffset, int distanceStride, const float* worldToClipMatrix) {
	// LEFT
	normals[normalOffset] = worldToClipMatrix[3] + worldToClipMatrix[0];
	normals[normalOffset + 1] = worldToClipMatrix[7] + worldToClipMatrix[4];
	normals[normalOffset + 2] = worldToClipMatrix[11] + worldToClipMatrix[8];
	distances[distanceOffset] = worldToClipMatrix[15] + worldToClipMatrix[12];

	// RIGHT
	normals[normalStride + normalOffset] = worldToClipMatrix[3] - worldToClipMatrix[0];
	normals[normalStride + normalOffset + 1] = worldToClipMatrix[7] - worldToClipMatrix[4];
	normals[normalStride + normalOffset + 2] = worldToClipMatrix[11] - worldToClipMatrix[8];
	distances[distanceStride + distanceOffset] = worldToClipMatrix[15] - worldToClipMatrix[12];

	// DOWN
	normals[normalStride * 2 + normalOffset] = worldToClipMatrix[3] + worldToClipMatrix[1];
	normals[normalStride * 2 + normalOffset + 1] = worldToClipMatrix[7] + worldToClipMatrix[5];
	normals[normalStride * 2 + normalOffset + 2] = worldToClipMatrix[11] + worldToClipMatrix[9];
	distances[distanceStride * 2 + distanceOffset] = worldToClipMatrix[15] + worldToClipMatrix[13];

	// TOP
	normals[normalStride * 3 + normalOffset] = worldToClipMatrix[3] - worldToClipMatrix[1];
	normals[normalStride * 3 + normalOffset + 1] = worldToClipMatrix[7] - worldToClipMatrix[5];
	normals[normalStride * 3 + normalOffset + 2] = worldToClipMatrix[11] - worldToClipMatrix[9];
	distances[distanceStride * 3 + distanceOffset] = worldToClipMatrix[15] - worldToClipMatrix[13];

	// NEAR
	normals[normalStride * 4 + normalOffset] = worldToClipMatrix[2];
	normals[normalStride * 4 + normalOffset + 1] = worldToClipMatrix[6];
	normals[normalStride * 4 + normalOffset + 2] = worldToClipMatrix[10];
	distances[distanceStride * 4 + distanceOffset] = worldToClipMatrix[14];

	// FAR
	normals[normalStride * 5 + normalOffset] = worldToClipMatrix[3] - worldToClipMatrix[2];
	normals[normalStride * 5 + normalOffset + 1] = worldToClipMatrix[7] - worldToClipMatrix[6];
	normals[normalStride * 5 + normalOffset + 2] = worldToClipMatrix[11] - worldToClipMatrix[10];
	distances[distanceStride * 5 + distanceOffset] = worldToClipMatrix[15] - worldToClipMatrix[14];
}

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

void GeometryUtility::GetSphereCoordinates(std::vector<glm::vec3>& points, std::vector<uint>& indexes, const glm::ivec2& resolution) {
	// step size between U-points on the grid
	glm::vec2 step = glm::vec2(Math::Pi2, Math::Pi) / glm::vec2(resolution);

	for (float i = 0; i < resolution.x; ++i) { // U-points
		for (float j = 0; j < resolution.y; ++j) { // V-points
			glm::vec2 uv = glm::vec2(i, j) * step;
			float un = ((i + 1) == resolution.x) ? Math::Pi2 : (i + 1) * step.x;
			float vn = ((j + 1) == resolution.y) ? Math::Pi : (j + 1) * step.y;

			// Find the four points of the grid square by evaluating the parametric urface function.
			glm::vec3 p[] = {
				SphereCoodrinate(uv.x, uv.y),
				SphereCoodrinate(uv.x, vn),
				SphereCoodrinate(un, uv.y),
				SphereCoodrinate(un, vn)
			};

			uint c = points.size();
			ADD_TRIANGLE(indexes, c + 0, c + 2, c + 1);
			ADD_TRIANGLE(indexes, c + 3, c + 1, c + 2);

			points.insert(points.end(), p, p + SUEDE_COUNTOF(p));
		}
	}
}

void GeometryUtility::GetCircleCoordinates(std::vector<glm::vec3>& points, const glm::vec3& center, float radius, const glm::vec3& normal, uint resolution) {
	float step = Math::Pi2 / resolution;
	glm::vec3 forward(0, 1, 0);
	if (!Math::Approximately(normal.y, 0) || !Math::Approximately(normal.z, 0)) {
		forward = glm::vec3(1, 0, 0);
	}

	forward = glm::normalize(glm::cross(forward, normal));

	for (int i = 0; i < resolution; ++i) {
		glm::quat q = glm::angleAxis(i * step, normal);
		points.push_back(q * forward * radius + center);
	}
}

void GeometryUtility::GetConeCoordinates(std::vector<glm::vec3>& points, std::vector<uint>& indexes, const glm::vec3& from, const glm::vec3& to, float radius, uint resolution) {
	glm::vec3 normal = glm::normalize(to - from);
	GetCircleCoordinates(points, from, radius, normal, resolution);

	int last = points.size();
	points.insert(points.end(), { from, to });

	for (int i = 1; i < last; ++i) {
		ADD_TRIANGLE(indexes, last + 1, i - 1, i);
		ADD_TRIANGLE(indexes, last, i, i - 1);
	}

	ADD_TRIANGLE(indexes, last, last - 1, 0);
	ADD_TRIANGLE(indexes, last + 1, 0, last - 1);
}

void GeometryUtility::GetCylinderCoordinates(std::vector<glm::vec3>& points, std::vector<uint>& indexes, const glm::vec3& from, const glm::vec3& to, float radius, uint resolution) 	{
	glm::vec3 dir = to - from;
	GetCircleCoordinates(points, from, radius, glm::normalize(dir), resolution);

	int last = points.size();
	for (int i = 0; i < last; ++i) {
		points.push_back(points[i] + dir);
	}

	int last2 = points.size();
	points.insert(points.end(), { from, to });

	for (int i = 1; i < last; ++i) {
		ADD_TRIANGLE(indexes, i - 1, last + i - 1, i);
		ADD_TRIANGLE(indexes, i, last + i - 1, last + i);
		ADD_TRIANGLE(indexes, last2, i - 1, i);
		ADD_TRIANGLE(indexes, last2 + 1, last + i - 1, last + i);
	}

	ADD_TRIANGLE(indexes, last - 1, last2 - 1, last);
	ADD_TRIANGLE(indexes, 0, last - 1, last);
	ADD_TRIANGLE(indexes, last2, last - 1, 0);
	ADD_TRIANGLE(indexes, last2 + 1, last2 - 1, last);
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
		if (!Math::Approximately(f, 0) && f < 0) {
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

glm::vec3 SphereCoodrinate(float x, float y) {
	return glm::vec3(Math::Cos(x) * Math::Sin(y), Math::Cos(y), Math::Sin(x) * Math::Sin(y));
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
	if (Math::Approximately(glm::dot(c, c), 0)) {
		return false;
	}

	for (array_list<EarVertex>::iterator ite = vertices.begin(); ite != vertices.end(); ++ite) {
		if (ite->index() == current || ite->index() == prev || ite->index() == next) {
			continue;
		}

		if (GeometryUtility::PolygonContains(points, SUEDE_COUNTOF(points), vertices[ite->index()].position, normal)) {
			return false;
		}
	}

	return true;
}

bool IsReflex(array_list<EarVertex>& vertices, int index, const glm::vec3& normal) {
	glm::vec3 current = vertices[index].position;
	glm::vec3 prev = vertices.prev_value(index).position;
	glm::vec3 next = vertices.next_value(index).position;
	return glm::angle(glm::normalize(next - current), glm::normalize(prev - current), normal) < 0;
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

		ADD_TRIANGLE(triangles, prevVertex.position, earTipVertex.position, nextVertex.position);

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
