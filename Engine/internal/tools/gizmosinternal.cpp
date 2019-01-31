#include "gizmosinternal.h"

#include <glm/gtc/matrix_transform.hpp>

#include "engine.h"
#include "graphics.h"
#include "resources.h"
#include "geometries.h"
#include "builtinproperties.h"

#include "memory/memory.h"

Gizmos::Gizmos() : singleton2<Gizmos>(MEMORY_NEW(GizmosInternal), Memory::DeleteRaw<GizmosInternal>) {}
void Gizmos::Flush() { _suede_dinstance()->Flush(); }
glm::mat4 Gizmos::GetMatrix() { return _suede_dinstance()->GetMatrix(); }
void Gizmos::SetMatrix(const glm::mat4& value) { _suede_dinstance()->SetMatrix(value); }
Color Gizmos::GetColor() { return _suede_dinstance()->GetColor(); }
void Gizmos::SetColor(const Color& value) { _suede_dinstance()->SetColor(value); }
void Gizmos::DrawLines(const glm::vec3* points, uint npoints) { _suede_dinstance()->DrawLines(points, npoints); }
void Gizmos::DrawLines(const std::initializer_list<glm::vec3>& points) { _suede_dinstance()->DrawLines(points.begin(), points.size()); }
void Gizmos::DrawLines(const glm::vec3* points, uint npoints, const uint* indexes, uint nindexes) { _suede_dinstance()->DrawLines(points, npoints, indexes, nindexes); }
void Gizmos::DrawLines(const std::initializer_list<glm::vec3>& points, const std::initializer_list<uint>& indexes) { _suede_dinstance()->DrawLines(points.begin(), points.size(), indexes.begin(), indexes.size()); }
void Gizmos::DrawLineStripe(const glm::vec3* points, uint npoints) { _suede_dinstance()->DrawLineStripe(points, npoints); }
void Gizmos::DrawLineStripe(const glm::vec3* points, uint npoints, const uint* indexes, uint nindexes) { _suede_dinstance()->DrawLineStripe(points, npoints, indexes, nindexes); }
void Gizmos::DrawCircle(const glm::vec3& center, float radius, const glm::vec3& normal) { _suede_dinstance()->DrawCircle(center, radius, normal); }
void Gizmos::DrawSphere(const glm::vec3& center, float radius) { _suede_dinstance()->DrawSphere(center, radius); }
void Gizmos::DrawCuboid(const glm::vec3& center, const glm::vec3& size) { _suede_dinstance()->DrawCuboid(center, size); }
void Gizmos::DrawWireSphere(const glm::vec3& center, float radius) { _suede_dinstance()->DrawWireSphere(center, radius); }
void Gizmos::DrawWireCuboid(const glm::vec3& center, const glm::vec3& size) { _suede_dinstance()->DrawWireCuboid(center, size); }

GizmosInternal::GizmosInternal() : color_(0, 1, 0, 1), matrix_(1) {
	mesh_ = new IMesh();
	mesh_->AddSubMesh(new ISubMesh());

	lineMaterial_ = new IMaterial();
	lineMaterial_->SetShader(Resources::FindShader("builtin/gizmos"));
	lineMaterial_->SetMatrix4("localToWorldMatrix", glm::mat4(1));

	Geometries::Sphere(sphere_.points, sphere_.indexes, glm::vec3(0), 1, glm::ivec2(15));

	Engine::AddFrameEventListener(this);
}

bool GizmosInternal::IsBatchable(const Batch& ref, MeshTopology topology, bool wireframe, Material material) {
	return ref.topology == topology && ref.wireframe == wireframe && ref.color == color_ && ref.material == material;
}

GizmosInternal::Batch& GizmosInternal::GetBatch(MeshTopology topology, bool wireframe, Material material) {
	if (batches_.empty() || !IsBatchable(batches_.back(), topology, wireframe, material)) {
		Batch b = { topology, wireframe, color_, material };
		batches_.push_back(b);
	}

	return batches_.back();
}

void GizmosInternal::DrawLines(const glm::vec3* points, uint npoints) {
	FillBatch(GetBatch(MeshTopology::Lines, true, lineMaterial_), points, npoints);
}

void GizmosInternal::DrawLines(const glm::vec3* points, uint npoints, const uint* indexes, uint nindexes) {
	FillBatch(GetBatch(MeshTopology::Lines, true, lineMaterial_), points, npoints, indexes, nindexes);
}

void GizmosInternal::DrawLineStripe(const glm::vec3* points, uint npoints) {
	FillBatch(GetBatch(MeshTopology::LineStripe, true, lineMaterial_), points, npoints);
}

void GizmosInternal::DrawLineStripe(const glm::vec3* points, uint npoints, const uint* indexes, uint nindexes) {
	FillBatch(GetBatch(MeshTopology::LineStripe, true, lineMaterial_), points, npoints, indexes, nindexes);
}

void GizmosInternal::DrawSphere(const glm::vec3& center, float radius) {
	AddSphereBatch(center, radius, false);
}

void GizmosInternal::DrawCuboid(const glm::vec3& center, const glm::vec3& size) {
	AddCuboidBatch(center, size, false);
}

void GizmosInternal::DrawCircle(const glm::vec3& center, float radius, const glm::vec3& normal) {
	std::vector<glm::vec3> points;
	Geometries::Circle(points, center, radius, normal, 36);
	points.push_back(points.front());

	FillBatch(GetBatch(MeshTopology::LineStripe, true, lineMaterial_), &points[0], points.size());
}

void GizmosInternal::DrawWireSphere(const glm::vec3& center, float radius) {
	AddSphereBatch(center, radius, true);
}

void GizmosInternal::DrawWireCuboid(const glm::vec3& center, const glm::vec3& size) {
	AddCuboidBatch(center, size, true);
}

void GizmosInternal::Flush() {
	for (Batch& b : batches_) {
		DrawGizmos(b);
	}

	batches_.clear();
}

void GizmosInternal::FillBatch(Batch& b, const glm::vec3* points, uint npoints, const uint* indexes, uint nindexes) {
	uint base = b.points.size();
	//b.points.reserve(base + npoints);

	for (uint i = 0; i < npoints; ++i) {
		b.points.push_back((matrix_ * glm::vec4(points[i], 1)).xyz);
	}

	//b.indexes.reserve(b.indexes.size() + nindexes);
	for (uint i = 0; i < nindexes; ++i) {
		b.indexes.push_back(base + indexes[i]);
	}
}

void GizmosInternal::FillBatch(Batch& b, const Geometry& geomety) {
	FillBatch(b, &geomety.points[0], geomety.points.size(), &geomety.indexes[0], geomety.indexes.size());
}

void GizmosInternal::FillBatch(Batch &b, const glm::vec3* points, uint npoints) {
	uint base = b.points.size();
	//b.points.reserve(base + npoints);

	for (uint i = 0; i < npoints; ++i) {
		b.points.push_back((matrix_ * glm::vec4(points[i], 1)).xyz);
	}

	//b.indexes.reserve(b.indexes.size() + npoints);
	for (uint i = 0; i < npoints; ++i) {
		b.indexes.push_back(base + i);
	}
}

/**
 * @see https://stackoverflow.com/questions/7687148/drawing-sphere-in-opengl-without-using-glusphere
 */
void GizmosInternal::AddSphereBatch(const glm::vec3& center, float radius, bool wireframe) {
	Material material = new IMaterial();
	material->SetShader(Resources::FindShader("builtin/gizmos"));
	material->SetMatrix4("localToWorldMatrix", glm::trs(center, glm::quat(), glm::vec3(radius)));

	FillBatch(GetBatch(MeshTopology::Triangles, wireframe, material), sphere_);
}

void GizmosInternal::AddCuboidBatch(const glm::vec3& center, const glm::vec3& size, bool wireframe) {
	std::vector<uint> indexes;
	std::vector<glm::vec3> points;
	Geometries::Cuboid(points, indexes, center, size);

	FillBatch(GetBatch(MeshTopology::Triangles, wireframe, lineMaterial_), &points[0], points.size(), &indexes[0], indexes.size());
}

void GizmosInternal::AddArrowBatch(const glm::vec3& from, const glm::vec3& to) {
}

void GizmosInternal::DrawGizmos(const Batch& b) {
	ShadingMode oldShadingMode = Graphics::GetShadingMode();
	Graphics::SetShadingMode(b.wireframe ? ShadingMode::Wireframe : ShadingMode::Shaded);

	MeshAttribute attribute;
	attribute.topology = b.topology;

	attribute.positions = b.points;
	attribute.indexes = b.indexes;

	mesh_->SetAttribute(attribute);

	TriangleBias bias{ b.indexes.size(), 0, 0 };
	mesh_->GetSubMesh(0)->SetTriangleBias(bias);

	b.material->SetColor(BuiltinProperties::MainColor, b.color);
	Graphics::Draw(mesh_, b.material);

	Graphics::SetShadingMode(oldShadingMode);
}
