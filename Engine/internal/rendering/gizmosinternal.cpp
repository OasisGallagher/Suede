#include "gizmosinternal.h"

#include "engine.h"
#include "graphics.h"
#include "resources.h"
#include "math/mathf.h"
#include "geometryutility.h"
#include "builtinproperties.h"

#include "memory/refptr.h"

Gizmos::Gizmos(Graphics* graphics) : Subsystem(new GizmosInternal(graphics)) {}
void Gizmos::Awake() { _suede_dptr()->Awake(); }
void Gizmos::Update(float deltaTime) { _suede_dptr()->Flush(); }
Matrix4 Gizmos::GetMatrix() { return _suede_dptr()->GetMatrix(); }
void Gizmos::SetMatrix(const Matrix4& value) { _suede_dptr()->SetMatrix(value); }
Color Gizmos::GetColor() { return _suede_dptr()->GetColor(); }
void Gizmos::SetColor(const Color& value) { _suede_dptr()->SetColor(value); }
void Gizmos::DrawLines(const Vector3* points, uint npoints) { _suede_dptr()->DrawLines(points, npoints); }
void Gizmos::DrawLines(const std::initializer_list<Vector3>& points) { _suede_dptr()->DrawLines(points.begin(), points.size()); }
void Gizmos::DrawLines(const Vector3* points, uint npoints, const uint* indexes, uint nindexes) { _suede_dptr()->DrawLines(points, npoints, indexes, nindexes); }
void Gizmos::DrawLines(const std::initializer_list<Vector3>& points, const std::initializer_list<uint>& indexes) { _suede_dptr()->DrawLines(points.begin(), points.size(), indexes.begin(), indexes.size()); }
void Gizmos::DrawLineStripe(const Vector3* points, uint npoints) { _suede_dptr()->DrawLineStripe(points, npoints); }
void Gizmos::DrawLineStripe(const Vector3* points, uint npoints, const uint* indexes, uint nindexes) { _suede_dptr()->DrawLineStripe(points, npoints, indexes, nindexes); }
void Gizmos::DrawSphere(const Vector3& center, float radius) { _suede_dptr()->DrawSphere(center, radius); }
void Gizmos::DrawCuboid(const Vector3& center, const Vector3& size) { _suede_dptr()->DrawCuboid(center, size); }
void Gizmos::DrawWireSphere(const Vector3& center, float radius) { _suede_dptr()->DrawWireSphere(center, radius); }
void Gizmos::DrawWireCuboid(const Vector3& center, const Vector3& size) { _suede_dptr()->DrawWireCuboid(center, size); }
void Gizmos::DrawArrow(const Vector3& from, const Vector3& to) { _suede_dptr()->DrawArrow(from, to); }

GizmosInternal::GizmosInternal(Graphics* graphics) : graphics_(graphics), color_(0, 1, 0, 1), matrix_(1) {
}

GizmosInternal::~GizmosInternal() {
}

void GizmosInternal::Awake() {
	mesh_ = new Mesh();
	mesh_->SetGeometry(new Geometry);

	lineMaterial_ = new Material();
	lineMaterial_->SetShader(Shader::Find("builtin/gizmos"));
	lineMaterial_->SetMatrix4("localToWorldMatrix", Matrix4(1));
}


bool GizmosInternal::IsBatchable(const Batch& ref, MeshTopology topology, Material* material) {
	return ref.topology == topology && ref.color == color_ && ref.material == material;
}

GizmosInternal::Batch& GizmosInternal::GetBatch(MeshTopology topology, Material* material) {
	if (batches_.empty() || !IsBatchable(batches_.back(), topology, material)) {
		Batch b = { topology, color_, material };
		batches_.push_back(b);
	}

	return batches_.back();
}

void GizmosInternal::DrawLines(const Vector3* points, uint npoints) {
	FillBatch(GetBatch(MeshTopology::Lines, lineMaterial_.get()), points, npoints);
}

void GizmosInternal::DrawLines(const Vector3* points, uint npoints, const uint* indexes, uint nindexes) {
	FillBatch(GetBatch(MeshTopology::Lines, lineMaterial_.get()), points, npoints, indexes, nindexes);
}

void GizmosInternal::DrawLineStripe(const Vector3* points, uint npoints) {
	FillBatch(GetBatch(MeshTopology::LineStripe, lineMaterial_.get()), points, npoints);
}

void GizmosInternal::DrawLineStripe(const Vector3* points, uint npoints, const uint* indexes, uint nindexes) {
	FillBatch(GetBatch(MeshTopology::LineStripe, lineMaterial_.get()), points, npoints, indexes, nindexes);
}

void GizmosInternal::DrawSphere(const Vector3& center, float radius) {
	AddSphereBatch(center, radius, false);
}

void GizmosInternal::DrawCuboid(const Vector3& center, const Vector3& size) {
	AddCuboidBatch(center, size, false);
}

void GizmosInternal::DrawWireSphere(const Vector3& center, float radius) {
	AddSphereBatch(center, radius, true);
}

void GizmosInternal::DrawWireCuboid(const Vector3& center, const Vector3& size) {
	AddCuboidBatch(center, size, true);
}

void GizmosInternal::DrawArrow(const Vector3& from, const Vector3& to) {
	uint kConeSlices = 16;
	float kConeHeight = 5, kConeRadius = 1, kMinmumLineLength = 10;

	Vector3 dest = to;
	Vector3 direction(to - from);
	if (direction.GetMagnitude() < kConeHeight) {
		dest = direction.GetNormalized() * (kConeHeight + kMinmumLineLength) + from;
		direction = dest - from;
	}

	AddConeBatch(direction.GetNormalized(), dest, kConeHeight, kConeRadius, kConeSlices);

	Vector3 segment[] = { from, from + direction.GetNormalized() * (direction.GetMagnitude() - kConeHeight) };
	FillBatch(GetBatch(MeshTopology::Lines, lineMaterial_.get()), segment, SUEDE_COUNTOF(segment));
}

Vector3 GizmosInternal::GetPerpendicular(const Vector3 &v) {
	float min = fabs(v.x);
	Vector3 cardinalAxis(1, 0, 0);

	if (fabs(v.y) < min) {
		min = fabs(v.y);
		cardinalAxis = Vector3(0, 1, 0);
	}

	if (fabs(v.z) < min) {
		cardinalAxis = Vector3(0, 0, 1);
	}

	return Vector3::Cross(v, cardinalAxis);
}

// http://www.freemancw.com/2012/06/opengl-cone-function/
void GizmosInternal::AddConeBatch(const Vector3 &direction, const Vector3 &apex, float coneHeight, float coneRadius, uint slices) {
	Vector3 c = apex + (-direction * coneHeight);

	std::vector<uint> indexes;
	std::vector<Vector3> points{ apex };

	// calculate points around directrix
	Vector3 e0 = GetPerpendicular(direction);
	Vector3 e1 = Vector3::Cross(e0, direction);
	float angInc = 360.f / slices * Mathf::deg2Rad;

	for (int i = 0; i < slices; ++i) {
		float rad = angInc * i;
		Vector3 p = c + (((e0 * cosf(rad)) + (e1 * sinf(rad))) * coneRadius);
		points.push_back(p);
	}

	// draw cone top
	for (uint i = 2; i < slices + 1; ++i) {
		indexes.insert(indexes.end(), { 0, i - 1,i });
	}

	indexes.insert(indexes.end(), { 0, slices, 1 });

	// draw cone bottom
	points.push_back(c);

	for (uint i = 2; i < slices + 1; ++i) {
		indexes.insert(indexes.end(), { slices + 1, i - 1, i });
	}

	indexes.insert(indexes.end(), { slices + 1, slices, 1 });

	FillBatch(GetBatch(MeshTopology::Triangles, lineMaterial_.get()), &points[0], points.size(), &indexes[0], indexes.size());
}

void GizmosInternal::Flush() {
	for (Batch& b : batches_) {
		DrawGizmos(b);
	}

	batches_.clear();
}

void GizmosInternal::FillBatch(Batch& b, const Vector3* points, uint npoints, const uint* indexes, uint nindexes) {
	uint base = b.points.size();
	//b.points.reserve(base + npoints);

	for (uint i = 0; i < npoints; ++i) {
		Vector4 p = matrix_ * Vector4(points[i].x, points[i].y, points[i].z, 1);
		b.points.push_back(Vector3(p.x, p.y, p.z));
	}

	//b.indexes.reserve(b.indexes.size() + nindexes);
	for (uint i = 0; i < nindexes; ++i) {
		b.indexes.push_back(base + indexes[i]);
	}
}

void GizmosInternal::FillBatch(Batch &b, const Vector3* points, uint npoints) {
	uint base = b.points.size();
	//b.points.reserve(base + npoints);

	for (uint i = 0; i < npoints; ++i) {
		Vector4 p = matrix_ * Vector4(points[i].x, points[i].y, points[i].z, 1);
		b.points.push_back(Vector3(p.x, p.y, p.z));
	}

	//b.indexes.reserve(b.indexes.size() + npoints);
	for (uint i = 0; i < npoints; ++i) {
		b.indexes.push_back(base + i);
	}
}

/**
* @see https://stackoverflow.com/questions/7687148/drawing-sphere-in-opengl-without-using-glusphere
*/
void GizmosInternal::AddSphereBatch(const Vector3& center, float radius, bool wireframe) {
	std::vector<uint> indexes;
	std::vector<Vector3> points;
	GeometryUtility::GetSphereCoodrinates(points, indexes, Vector2(15));

	ref_ptr<Material> material = new Material();
	material->SetShader(Shader::Find("builtin/gizmos"));
	material->SetMatrix4("localToWorldMatrix", Matrix4::TRS(center, Quaternion(), Vector3(radius)));

	MeshTopology topology = MeshTopology::Triangles;
	if (wireframe) {
		topology = MeshTopology::Lines;
		indexes.clear();
		for (uint i = 0; i < points.size(); i += 4) {
			indexes.insert(indexes.end(), {
				i, i + 1, i + 1, i + 2, i + 2, i + 3,i + 3, i
			});
		}
	}

	FillBatch(GetBatch(topology, material.get()), &points[0], points.size(), &indexes[0], indexes.size());
}

void GizmosInternal::AddCuboidBatch(const Vector3& center, const Vector3& size, bool wireframe) {
	std::vector<uint> indexes;
	std::vector<Vector3> points;
	GeometryUtility::GetCuboidCoordinates(points, center, size, &indexes);

	MeshTopology topology = MeshTopology::Triangles;
	if (wireframe) {
		topology = MeshTopology::Lines;
		indexes.assign({
			0, 1, 1, 2, 2, 3, 3, 0,
			4, 5, 5, 6, 6, 7, 7, 4,
			0, 4, 1, 5, 2, 6, 3, 7
		});
	}

	FillBatch(GetBatch(topology, lineMaterial_.get()), &points[0], points.size(), &indexes[0], indexes.size());
}

void GizmosInternal::DrawGizmos(const Batch& b) {
	Geometry* geometry = mesh_->GetGeometry();
	geometry->SetTopology(b.topology);
	geometry->SetVertices(b.points.data(), b.points.size());
	geometry->SetIndexes(b.indexes.data(), b.indexes.size());

	if (mesh_->GetSubMeshCount() == 0) {
		mesh_->AddSubMesh(new SubMesh());
	}

	TriangleBias bias{ b.indexes.size(), 0, 0 };
	mesh_->GetSubMesh(0)->SetTriangleBias(bias);

	b.material->SetColor(BuiltinProperties::MainColor, b.color);
	graphics_->Draw(mesh_.get(), b.material.get());
}
