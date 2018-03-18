#include "plane.h"
#include "tools/math2.h"
#include "debug/debug.h"
#include "geometryutility.h"

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

bool GeometryUtility::DiagonalRectContains(const glm::vec3& point, const glm::vec3& tl, const glm::vec3& rb) {
	float xMin = tl.x, xMax = rb.x;
	if (xMin > xMax) { float tmp = xMin; xMin = xMax; xMax = tmp; }

	float zMin = tl.z, zMax = rb.z;
	if (zMin > zMax) { float tmp = zMin; zMin = zMax; zMax = tmp; }

	return point.x >= xMin && point.x <= xMax && point.z >= zMin && point.z <= zMax;
}

bool GeometryUtility::PolygonContains(const glm::vec3* positions, uint npositions, const glm::vec3& point, const glm::vec3& normal, bool onEdge) {
	for (uint i = 1; i <= npositions; ++i) {
		const glm::vec3& currentPosition = i < npositions ? positions[i] : positions[0];
		float cr = Math::Angle(glm::normalize(currentPosition - positions[i - 1]), glm::normalize(point - positions[i - 1]), normal);
		if (IsZero(cr) && DiagonalRectContains(point, currentPosition, positions[i - 1])) {
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

bool GeometryUtility::IsFrontFace(const Triangle& triangle, const glm::vec3& camera) {
	glm::vec3 normal = glm::cross(triangle[1] - triangle[0], triangle[2] - triangle[1]);
	return glm::dot(normal, triangle[1] - camera) < 0;
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

Side GeometryUtility::CalculateSide(const Plane& plane, const glm::vec3* points, uint npoints) {
	uint npositive = 0, nnegative = 0;
	for (uint j = 0; j < npoints; ++j) {
		float f = GeometryUtility::GetDistance(plane, points[j]);
		if (f < 0) { ++nnegative; }
		else if (f > 0) { ++npositive; }
	}

	if (npositive == 0 && nnegative == 0) { return SideCoinciding; }

	if (npositive == 0) { return SideBehind; }

	if (nnegative == 0) { return SideInfront; }

	return SideSpanning;
}

uint GeometryUtility::CountPointsNotBehindPlanes(const Plane* planes, uint nplanes, const glm::vec3* points, uint npoints) {
	uint count = 0;
	for (uint i = 0; i < npoints; ++i) {
		bool inside = true;
		for (uint j = 0; j < nplanes; ++j) {
			if (GeometryUtility::CalculateSide(planes[j], points + i, 1) == SideBehind) {
				inside = false;
			}
		}

		if (inside) { ++count; }
	}

	return count;
}

bool GeometryUtility::GetUniqueIntersection(glm::vec3& intersection, const Plane& plane, const glm::vec3& prev, const glm::vec3& next) {
	if (!GetIntersection(intersection, plane, prev, next)) {
		return false;
	}

	if (prev == intersection || next == intersection) {
		Debug::LogWarning("line start or end");
		return false;
	}

	return true;
}

void GeometryUtility::RemovePointsBehindPlane(std::list<glm::vec3>& list, const Plane& plane) {
	for (std::list<glm::vec3>::iterator current = list.begin(); current != list.end();) {
		float f = GetDistance(plane, *current);
		if (!IsZero(f) && f < 0) {
			current = list.erase(current);
		}
		else {
			++current;
		}
	}
}

void GeometryUtility::EarClippingTriangulate(std::vector<glm::vec3>& triangles, const std::vector<glm::vec3>& polygon, const glm::vec3& normal) {
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

void GeometryUtility::ClampPolygon(std::list<glm::vec3>& list, const Plane& plane) {
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

bool GeometryUtility::IsEar(array_list<EarVertex>& vertices, int current, const glm::vec3& normal) {
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

		if (PolygonContains(points, CountOf(points), vertices[ite->index()].position, normal)) {
			return false;
		}
	}

	return true;
}

bool GeometryUtility::IsReflex(array_list<EarVertex>& vertices, int index, const glm::vec3& normal) {
	glm::vec3 current = vertices[index].position;
	glm::vec3 prev = vertices.prev_value(index).position;
	glm::vec3 next = vertices.next_value(index).position;
	return Math::Angle(glm::normalize(next - current), glm::normalize(prev - current), normal) < 0;
}

void GeometryUtility::EarClipping(std::vector<glm::vec3>& triangles, array_list<EarVertex>& vertices, array_list<int>& earTips, const glm::vec3& normal) {
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

int GeometryUtility::UpdateEarVertexState(array_list<EarVertex>& vertices, int vertexIndex, const glm::vec3& normal) {
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
