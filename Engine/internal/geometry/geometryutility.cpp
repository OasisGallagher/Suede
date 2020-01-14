#include <list>

#include "math/mathf.h"
#include "math/vector2.h"
#include "geometryutility.h"
#include "containers/arraylist.h"

#define IsZero(f)	(fabs((f)) < 1e-5)

struct EarVertex {
	enum { Reflex = 1, Ear = 2, };

	EarVertex() : mask(0), earListIndex(-1) {}
	EarVertex(const Vector3& value) : EarVertex() { position = value; }

	bool SetMask(int value, bool addMask);
	bool TestMask(int value) const { return (mask & value) != 0; }

	int mask;
	int earListIndex;
	Vector3 position;
};

static Vector3 SphereCoodrinate(float x, float y);

static void ClampPolygon(std::list<Vector3>& list, const Plane& plane);
static void RemovePointsBehindPlane(std::list<Vector3>& list, const Plane& plane);

static uint CountPointsNotBehindPlanes(const Plane* planes, uint nplanes, const Vector3* points, uint npoints);
static bool GetUniqueIntersection(Vector3& intersection, const Plane& plane, const Vector3& prev, const Vector3& next);

static bool IsEar(array_list<EarVertex>& vertices, int current, const Vector3& normal);
static bool IsReflex(array_list<EarVertex>& vertices, int index, const Vector3& normal);
static int UpdateEarVertexState(array_list<EarVertex>& vertices, int vertexIndex, const Vector3& normal);

static void EarClipping(std::vector<Vector3>& triangles, array_list<EarVertex>& vertices, array_list<int>& earTips, const Vector3& normal);
static void EarClippingTriangulate(std::vector<Vector3>& triangles, const std::vector<Vector3>& polygon, const Vector3& normal);

bool GeometryUtility::AARectContains(const Vector3& point, const Vector3& tl, const Vector3& rb) {
	float xMin = tl.x, xMax = rb.x;
	if (xMin > xMax) { float tmp = xMin; xMin = xMax; xMax = tmp; }

	float zMin = tl.z, zMax = rb.z;
	if (zMin > zMax) { float tmp = zMin; zMin = zMax; zMax = tmp; }

	return point.x >= xMin && point.x <= xMax && point.z >= zMin && point.z <= zMax;
}

bool GeometryUtility::PolygonContains(const Vector3* vertices, uint nvertices, const Vector3& point, const Vector3& normal, bool onEdge) {
	for (uint i = 1; i <= nvertices; ++i) {
		const Vector3& currentPosition = i < nvertices ? vertices[i] : vertices[0];
		float cr = Vector3::Angle((currentPosition - vertices[i - 1]).GetNormalized(), (point - vertices[i - 1]).GetNormalized(), normal);
		if (IsZero(cr) && AARectContains(point, currentPosition, vertices[i - 1])) {
			return onEdge;
		}

		if (cr > 0) {
			return false;
		}
	}

	return true;
}

void GeometryUtility::Triangulate(std::vector<Vector3>& triangles, const std::vector<Vector3>& polygon, const Vector3& normal) {
	if (polygon.size() == 3) {
		triangles.insert(triangles.end(), polygon.begin(), polygon.end());
	}
	else if (polygon.size() > 3) {
		EarClippingTriangulate(triangles, polygon, normal);
	}
}

void GeometryUtility::ClampTriangle(std::vector<Vector3>& polygon, const Triangle& triangle, const Plane* planes, uint count) {
	if (CountPointsNotBehindPlanes(planes, count, triangle.points, 3) == 3) {
		polygon.insert(polygon.end(), triangle.points, triangle.points + 3);
	}
	else {
		std::list<Vector3> list(triangle.points, triangle.points + 3);

		for (int pi = 0; list.size() >= 3 && pi < count; ++pi) {
			ClampPolygon(list, planes[pi]);
		}

		polygon.insert(polygon.end(), list.begin(), list.end());
	}
}

bool GeometryUtility::IsFrontFace(const Triangle& face, const Vector3& camera) {
	Vector3 normal = Vector3::Cross(face[1] - face[0], face[2] - face[1]);
	return Vector3::Dot(normal, face[1] - camera) < 0;
}

bool GeometryUtility::GetIntersection(Vector3& intersection, const Plane& plane, const Vector3& p0, const Vector3& p1) {
	float d0 = plane.GetDistanceToPoint(p0);
	float d1 = plane.GetDistanceToPoint(p1);

	if (IsZero(d0 * d1) || d0 * d1 >= 0) {
		return false;
	}

	float t = d0 / (d0 - d1);
	intersection = p0 + t * (p1 - p0);
	return true;
}

void extract_planes_from_projmat(Plane* planes,
	const Matrix4& mat) {
	float left[4], right[4], bottom[4], top[4], near[4], far[4];
	for (int i = 4; i--; ) left[i] = mat[i][3] + mat[i][0];
	for (int i = 4; i--; ) right[i] = mat[i][3] - mat[i][0];
	for (int i = 4; i--; ) bottom[i] = mat[i][3] + mat[i][1];
	for (int i = 4; i--; ) top[i] = mat[i][3] - mat[i][1];
	for (int i = 4; i--; ) near[i] = mat[i][3] + mat[i][2];
	for (int i = 4; i--; ) far[i] = mat[i][3] - mat[i][2];
}

void GeometryUtility::CalculateFrustumPlanes(Plane* planes, const Matrix4& worldToClipMatrix) {
#define EXTRACT_PLANE_IMPL(index, sign, row)	\
	planes[index] = Plane(Vector4( \
		worldToClipMatrix[0][3] sign worldToClipMatrix[0][row], \
		worldToClipMatrix[1][3] sign worldToClipMatrix[1][row], \
		worldToClipMatrix[2][3] sign worldToClipMatrix[2][row], \
		worldToClipMatrix[3][3] sign worldToClipMatrix[3][row]))

	EXTRACT_PLANE_IMPL(0, +, 0);
	EXTRACT_PLANE_IMPL(1, -, 0);
	EXTRACT_PLANE_IMPL(2, +, 1);
	EXTRACT_PLANE_IMPL(3, -, 1);
	EXTRACT_PLANE_IMPL(4, +, 2);
	EXTRACT_PLANE_IMPL(5, -, 2);

#undef EXTRACT_PLANE_IMPL
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

PlaneSide GeometryUtility::TestSide(const Plane& plane, const Vector3* points, uint npoints) {
	uint npositive = 0, nnegative = 0;
	for (uint j = 0; j < npoints; ++j) {
		float f = plane.GetDistanceToPoint(points[j]);
		if (f < 0) { ++nnegative; }
		else if (f > 0) { ++npositive; }
	}

	if (npositive == 0 && nnegative == 0) { return PlaneSide::Coinciding; }

	if (npositive == 0) { return PlaneSide::Behind; }

	if (nnegative == 0) { return PlaneSide::Infront; }

	return PlaneSide::Spanning;
}

void GeometryUtility::GetSphereCoodrinates(std::vector<Vector3>& points, std::vector<uint>& indexes, const Vector2& resolution) {
	// step size between U-points on the grid
	Vector2 step = Vector2(Mathf::pi * 2, Mathf::pi) / Vector2(resolution);

	for (float i = 0; i < resolution.x; ++i) { // U-points
		for (float j = 0; j < resolution.y; ++j) { // V-points
			Vector2 uv = Vector2(i, j) * step;
			float un = ((i + 1) == resolution.x) ? Mathf::pi * 2 : (i + 1) * step.x;
			float vn = ((j + 1) == resolution.y) ? Mathf::pi : (j + 1) * step.y;

			// Find the four points of the grid square by evaluating the parametric urface function.
			Vector3 p[] = {
				SphereCoodrinate(uv.x, uv.y),
				SphereCoodrinate(uv.x, vn),
				SphereCoodrinate(un, uv.y),
				SphereCoodrinate(un, vn)
			};

			uint c = points.size();
			indexes.push_back(c + 0);
			indexes.push_back(c + 2);
			indexes.push_back(c + 1);
			indexes.push_back(c + 3);
			indexes.push_back(c + 1);
			indexes.push_back(c + 2);

			points.insert(points.end(), p, p + SUEDE_COUNTOF(p));
		}
	}
}

void GeometryUtility::GetCuboidCoordinates(std::vector<Vector3>& points, const Vector3& center, const Vector3& size, std::vector<uint>* triangles) {
	Vector3 half = size / 2.f;

	points.assign({
		center + Vector3(half.x, half.y, -half.z),
		center + Vector3(-half.x, half.y, -half.z),
		center + Vector3(-half.x, half.y, half.z),
		center + half,
		center + Vector3(half.x, - half.y, -half.z),
		center + (-half),
		center + Vector3(- half.x, -half.y, half.z),
		center + Vector3(half.x, -half.y, half.z),
	});

	if (triangles != nullptr) {
		triangles->assign({
			0, 1, 2, 0, 2, 3, 0, 4, 5, 0, 5, 1,
			1, 5, 6, 1, 6, 2, 2, 6, 7, 2, 7, 3,
			3, 7, 4, 3, 4, 0, 4, 7, 6, 4, 6, 5
		});
	}
}

bool GeometryUtility::PlanesCulling(Plane* planes, uint nplanes, const Vector3* points, uint npoints) {
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

// https://www.braynzarsoft.net/viewtutorial/q16390-34-aabb-cpu-side-frustum-culling
bool GeometryUtility::FrustumIntersectsAABB(const Plane* frustum, const Bounds& bounds) {
	Vector3 min = bounds.GetMin(), max = bounds.GetMax();
	for (int i = 0; i < 6; i++) {
		const Plane& plane = frustum[i];
		const Vector3& normal = plane.GetNormal();

		Vector3 point(
			normal.x > 0 ? max.x : min.x,
			normal.y > 0 ? max.y : min.y,
			normal.z > 0 ? max.z : min.z
		);

		if (plane.GetDistanceToPoint(point) < 0) {
			return false;
		}
	}

	return true;
}

uint CountPointsNotBehindPlanes(const Plane* planes, uint nplanes, const Vector3* points, uint npoints) {
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

bool GetUniqueIntersection(Vector3& intersection, const Plane& plane, const Vector3& prev, const Vector3& next) {
	if (!GeometryUtility::GetIntersection(intersection, plane, prev, next)) {
		return false;
	}

	if (prev == intersection || next == intersection) {
		Debug::LogWarning("line start or end");
		return false;
	}

	return true;
}

void RemovePointsBehindPlane(std::list<Vector3>& list, const Plane& plane) {
	for (std::list<Vector3>::iterator current = list.begin(); current != list.end();) {
		float d = plane.GetDistanceToPoint(*current);
		if (!IsZero(d) && d < 0) {
			current = list.erase(current);
		}
		else {
			++current;
		}
	}
}

void EarClippingTriangulate(std::vector<Vector3>& triangles, const std::vector<Vector3>& polygon, const Vector3& normal) {
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

Vector3 SphereCoodrinate(float x, float y) {
	return Vector3(cosf(x) * sinf(y), cosf(y), sinf(x) * sinf(y));
}

void ClampPolygon(std::list<Vector3>& list, const Plane& plane) {
	Vector3 intersection;
	std::list<Vector3>::iterator prev = list.begin(), next = prev;

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

bool IsEar(array_list<EarVertex>& vertices, int current, const Vector3& normal) {
	if (vertices.size() < 3) { return false; }

	int prev = vertices.prev_index(current);
	int next = vertices.next_index(current);

	Vector3 points[] = {
		vertices[prev].position,
		vertices[current].position,
		vertices[next].position,
	};

	Vector3 c = Vector3::Cross(points[0] - points[1], points[2] - points[1]);
	// Collinear.
	if (IsZero(c.GetSqrMagnitude())) {
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

bool IsReflex(array_list<EarVertex>& vertices, int index, const Vector3& normal) {
	Vector3 current = vertices[index].position;
	Vector3 prev = vertices.prev_value(index).position;
	Vector3 next = vertices.next_value(index).position;
	return Vector3::Angle((next - current).GetNormalized(), (prev - current).GetNormalized(), normal) < 0;
}

void EarClipping(std::vector<Vector3>& triangles, array_list<EarVertex>& vertices, array_list<int>& earTips, const Vector3& normal) {
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

int UpdateEarVertexState(array_list<EarVertex>& vertices, int vertexIndex, const Vector3& normal) {
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
