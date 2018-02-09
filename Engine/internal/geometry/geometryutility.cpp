#include "plane.h"
#include "tools/math2.h"
#include "debug/debug.h"
#include "geometryutility.h"

#define IsZero(f)	(fabs((f)) < 1e-5)

struct EarVertex {
	enum { Reflex = 1, Ear = 2, };

	EarVertex() {}
	EarVertex(const glm::vec3& value) { position = value; }

	bool SetMask(int value, bool addMask);
	bool TestMask(int value) const { return (mask & value) != 0; }

	int mask;
	int earListIndex = -1;
	glm::vec3 position;
};

bool GeometryUtility::DiagonalRectContains(const glm::vec3& point, const glm::vec3& tl, const glm::vec3& rb) {
	float xMin = tl.x, xMax = rb.x;
	if (xMin > xMax) { float tmp = xMin; xMin = xMax; xMax = tmp; }

	float zMin = tl.z, zMax = rb.z;
	if (zMin > zMax) { float tmp = zMin; zMin = zMax; zMax = tmp; }

	return point.x >= xMin && point.x <= xMax && point.z >= zMin && point.z <= zMax;
}

bool GeometryUtility::PolygonContains(const std::vector<glm::vec3>& positions, const glm::vec3& point, const glm::vec3& normal, bool onEdge) {
	for (int i = 1; i <= positions.size(); ++i) {
		const glm::vec3& currentPosition = i < positions.size() ? positions[i] : positions[0];
		float cr = Math::Angle(glm::normalize(point - positions[i - 1]), glm::normalize(currentPosition - positions[i - 1]), normal);
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
	return EarClippingTriangulate(triangles, polygon, normal);
}

void GeometryUtility::ClampTriangle(std::vector<glm::vec3>& polygon, const glm::vec3 triangle[3], const Plane* planes, uint count) {
	std::list<glm::vec3> list(triangle, triangle + 3);

	for (int pi = 0; list.size() >= 3 && pi < count; ++pi) {
		ClampPolygon(list, planes[pi]);
	}

	polygon.insert(polygon.end(), list.begin(), list.end());
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

void GeometryUtility::CalculateFrustumPlanes(Plane(&planes)[6], const glm::mat4& worldToClipSpaceMatrix) {
	planes[0] = Plane(worldToClipSpaceMatrix[3] + worldToClipSpaceMatrix[0]);
	planes[1] = Plane(worldToClipSpaceMatrix[3] - worldToClipSpaceMatrix[0]);
	planes[2] = Plane(worldToClipSpaceMatrix[3] + worldToClipSpaceMatrix[1]);
	planes[3] = Plane(worldToClipSpaceMatrix[3] - worldToClipSpaceMatrix[1]);
	planes[4] = Plane(worldToClipSpaceMatrix[3] + worldToClipSpaceMatrix[2]);
	planes[5] = Plane(worldToClipSpaceMatrix[3] - worldToClipSpaceMatrix[2]);
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

	DoTriangulate(triangles, vertices, earTips, normal);
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

	std::vector<glm::vec3> points;
	points.push_back(vertices[prev].position);
	points.push_back(vertices[current].position);
	points.push_back(vertices[next].position);

	glm::vec3 c = glm::cross(points[0] - points[1], points[2] - points[1]);
	// Collinear.
	if (IsZero(glm::dot(c, c))) {
		return false;
	}

	for (array_list<EarVertex>::iterator ite = vertices.begin(); ite != vertices.end(); ++ite) {
		if (ite->index() == current || ite->index() == prev || ite->index() == next) {
			continue;
		}

		if (PolygonContains(points, vertices[ite->index()].position, normal)) {
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

void GeometryUtility::DoTriangulate(std::vector<glm::vec3>& triangles, array_list<EarVertex>& vertices, array_list<int>& earTips, const glm::vec3& normal) {
	triangles.reserve((vertices.size() - 2) * 3);

	EarVertex removedEars[2];
	int removedEarCount = 0;

	int earTipIndex = -1;
	for (array_list<int>::iterator ite = earTips.begin(); ite != earTips.end(); ++ite) {
		if (earTipIndex >= 0) { earTips.erase(earTipIndex); }

		earTipIndex = ite->index();

		int earTipVertexIndex = earTips[earTipIndex];
		EarVertex& earTipVertex = vertices[earTipVertexIndex];

		int prevIndex = vertices.prev_index(earTipVertexIndex);
		EarVertex& prevVertex = vertices.prev_value(earTipVertexIndex);

		int nextIndex = vertices.next_index(earTipVertexIndex);
		EarVertex nextVertex = vertices.next_value(earTipVertexIndex);

		triangles.push_back(prevVertex.position);
		triangles.push_back(earTipVertex.position);
		triangles.push_back(nextVertex.position);

		vertices.erase(earTipVertexIndex);

		int state = UpdateEarVertexState(vertices, prevIndex, normal);
		if (state > 0) {
			prevVertex.earListIndex = earTips.add(prevIndex);
		}
		else if (state < 0) {
			removedEars[removedEarCount++] = prevVertex;
		}

		state = UpdateEarVertexState(vertices, nextIndex, normal);
		if (state > 0) {
			nextVertex.earListIndex = earTips.add(nextIndex);
		}
		else if (state < 0) {
			removedEars[removedEarCount++] = nextVertex;
		}

		for (int i = 0; i < removedEarCount; ++i) {
			if (!(removedEars[i].earListIndex >= 0)) {
				Debug::LogError("");
			}

			earTips.erase(removedEars[i].earListIndex);
			removedEars[i].earListIndex = -1;
		}

		removedEarCount = 0;
	}

	if (earTipIndex >= 0) { earTips.erase(earTipIndex); }
}

int GeometryUtility::UpdateEarVertexState(array_list<EarVertex>& vertices, int vertexIndex, const glm::vec3& normal) {
	EarVertex earVertex = vertices[vertexIndex];

	int result = 0;

	bool isEar = earVertex.TestMask(EarVertex::Ear);

	if (earVertex.TestMask(EarVertex::Reflex)) {
		if (!(!isEar)) {
			Debug::LogError("");
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
