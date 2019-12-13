#include "gizmosinternal.h"

#include "engine.h"
#include "graphics.h"
#include "resources.h"
#include "math/mathf.h"
#include "geometryutility.h"
#include "frameeventqueue.h"
#include "builtinproperties.h"

#include "memory/refptr.h"

Gizmos::Gizmos() : Singleton2<Gizmos>(new GizmosInternal, t_delete<GizmosInternal>) {}
void Gizmos::Flush() { _suede_dinstance()->Flush(); }
Matrix4 Gizmos::GetMatrix() { return _suede_dinstance()->GetMatrix(); }
void Gizmos::SetMatrix(const Matrix4& value) { _suede_dinstance()->SetMatrix(value); }
Color Gizmos::GetColor() { return _suede_dinstance()->GetColor(); }
void Gizmos::SetColor(const Color& value) { _suede_dinstance()->SetColor(value); }
void Gizmos::DrawLines(const Vector3* points, uint npoints) { _suede_dinstance()->DrawLines(points, npoints); }
void Gizmos::DrawLines(const std::initializer_list<Vector3>& points) { _suede_dinstance()->DrawLines(points.begin(), points.size()); }
void Gizmos::DrawLines(const Vector3* points, uint npoints, const uint* indexes, uint nindexes) { _suede_dinstance()->DrawLines(points, npoints, indexes, nindexes); }
void Gizmos::DrawLines(const std::initializer_list<Vector3>& points, const std::initializer_list<uint>& indexes) { _suede_dinstance()->DrawLines(points.begin(), points.size(), indexes.begin(), indexes.size()); }
void Gizmos::DrawLineStripe(const Vector3* points, uint npoints) { _suede_dinstance()->DrawLineStripe(points, npoints); }
void Gizmos::DrawLineStripe(const Vector3* points, uint npoints, const uint* indexes, uint nindexes) { _suede_dinstance()->DrawLineStripe(points, npoints, indexes, nindexes); }
void Gizmos::DrawSphere(const Vector3& center, float radius) { _suede_dinstance()->DrawSphere(center, radius); }
void Gizmos::DrawCuboid(const Vector3& center, const Vector3& size) { _suede_dinstance()->DrawCuboid(center, size); }
void Gizmos::DrawWireSphere(const Vector3& center, float radius) { _suede_dinstance()->DrawWireSphere(center, radius); }
void Gizmos::DrawWireCuboid(const Vector3& center, const Vector3& size) { _suede_dinstance()->DrawWireCuboid(center, size); }

GizmosInternal::GizmosInternal() : color_(0, 1, 0, 1), matrix_(1) {
	mesh_ = new Mesh();

	lineMaterial_ = new Material();
	lineMaterial_->SetShader(Resources::FindShader("builtin/gizmos"));
	lineMaterial_->SetMatrix4("localToWorldMatrix", Matrix4(1));

	Engine::frameLeave.subscribe(this, &GizmosInternal::OnFrameLeave, (int)FrameEventQueue::Gizmos);
}

bool GizmosInternal::IsBatchable(const Batch& ref, MeshTopology topology, bool wireframe, Material* material) {
	return ref.topology == topology && ref.wireframe == wireframe && ref.color == color_ && ref.material == material;
}

GizmosInternal::Batch& GizmosInternal::GetBatch(MeshTopology topology, bool wireframe, Material* material) {
	if (batches_.empty() || !IsBatchable(batches_.back(), topology, wireframe, material)) {
		Batch b = { topology, wireframe, color_, material };
		batches_.push_back(b);
	}

	return batches_.back();
}

void GizmosInternal::DrawLines(const Vector3* points, uint npoints) {
	FillBatch(GetBatch(MeshTopology::Lines, true, lineMaterial_.get()), points, npoints);
}

void GizmosInternal::DrawLines(const Vector3* points, uint npoints, const uint* indexes, uint nindexes) {
	FillBatch(GetBatch(MeshTopology::Lines, true, lineMaterial_.get()), points, npoints, indexes, nindexes);
}

void GizmosInternal::DrawLineStripe(const Vector3* points, uint npoints) {
	FillBatch(GetBatch(MeshTopology::LineStripe, true, lineMaterial_.get()), points, npoints);
}

void GizmosInternal::DrawLineStripe(const Vector3* points, uint npoints, const uint* indexes, uint nindexes) {
	FillBatch(GetBatch(MeshTopology::LineStripe, true, lineMaterial_.get()), points, npoints, indexes, nindexes);
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

	Material* material = new Material();
	material->SetShader(Resources::FindShader("builtin/gizmos"));
	material->SetMatrix4("localToWorldMatrix", Matrix4::TRS(center, Quaternion(), Vector3(radius)));

	FillBatch(GetBatch(MeshTopology::Triangles, wireframe, material), &points[0], points.size(), &indexes[0], indexes.size());
}

void GizmosInternal::AddCuboidBatch(const Vector3& center, const Vector3& size, bool wireframe) {
	std::vector<uint> indexes;
	std::vector<Vector3> points;
	GeometryUtility::GetCuboidCoordinates(points, center, size, &indexes);

	FillBatch(GetBatch(MeshTopology::Triangles, wireframe, lineMaterial_.get()), &points[0], points.size(), &indexes[0], indexes.size());
}

void GizmosInternal::DrawGizmos(const Batch& b) {
	ShadingMode oldShadingMode = Graphics::GetShadingMode();
	Graphics::SetShadingMode(b.wireframe ? ShadingMode::Wireframe : ShadingMode::Shaded);

	MeshAttribute attribute;
	attribute.topology = b.topology;

	attribute.positions = b.points;
	attribute.indexes = b.indexes;

	mesh_->SetAttribute(attribute);

	if (mesh_->GetSubMeshCount() == 0) {
		mesh_->AddSubMesh(new SubMesh());
	}

	TriangleBias bias{ b.indexes.size(), 0, 0 };
	mesh_->GetSubMesh(0)->SetTriangleBias(bias);

	b.material->SetColor(BuiltinProperties::MainColor, b.color);
	Graphics::Draw(mesh_.get(), b.material.get());

	Graphics::SetShadingMode(oldShadingMode);
}
