#include "gizmosinternal.h"

#include "engine.h"
#include "graphics.h"
#include "resources.h"
#include "tools/math2.h"
#include "geometryutility.h"
#include "builtinproperties.h"

GizmosInternal::GizmosInternal() : color_(0, 1, 0, 1) {
	mesh_ = NewMesh();

	lineMaterial_ = NewMaterial();
	lineMaterial_->SetShader(Resources::instance()->FindShader("builtin/gizmos"));
	lineMaterial_->SetMatrix4("localToWorldMatrix", glm::mat4(1));

	Engine::instance()->AddFrameEventListener(this);
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
	b.points.insert(b.points.end(), points, points + npoints);

	for (uint i = 0; i < nindexes; ++i) {
		b.indexes.push_back(base + indexes[i]);
	}
}

void GizmosInternal::FillBatch(Batch &b, const glm::vec3* points, uint npoints) {
	uint base = b.points.size();
	b.points.insert(b.points.end(), points, points + npoints);
	for (uint i = 0; i < npoints; ++i) {
		b.indexes.push_back(base + i);
	}
}

/**
 * @see https://stackoverflow.com/questions/7687148/drawing-sphere-in-opengl-without-using-glusphere
 */
void GizmosInternal::AddSphereBatch(const glm::vec3& center, float radius, bool wireframe) {
	std::vector<uint> indexes;
	std::vector<glm::vec3> points;
	GeometryUtility::GetSphereCoodrinates(points, indexes, glm::ivec2(15));

	Material material = NewMaterial();
	material->SetShader(Resources::instance()->FindShader("builtin/gizmos"));
	material->SetMatrix4("localToWorldMatrix", Math::TRS(center, glm::quat(), glm::vec3(radius)));

	FillBatch(GetBatch(MeshTopology::Triangles, wireframe, material), &points[0], points.size(), &indexes[0], indexes.size());
}

void GizmosInternal::AddCuboidBatch(const glm::vec3& center, const glm::vec3& size, bool wireframe) {
	std::vector<uint> indexes;
	std::vector<glm::vec3> points;
	GeometryUtility::GetCuboidCoordinates(points, center, size, &indexes);

	FillBatch(GetBatch(MeshTopology::Triangles, wireframe, lineMaterial_), &points[0], points.size(), &indexes[0], indexes.size());
}

void GizmosInternal::DrawGizmos(const Batch& b) {
	ShadingMode oldShadingMode = Graphics::instance()->GetShadingMode();
	Graphics::instance()->SetShadingMode(b.wireframe ? ShadingMode::Wireframe : ShadingMode::Shaded);

	MeshAttribute attribute;
	attribute.topology = b.topology;

	attribute.positions = b.points;
	attribute.indexes = b.indexes;

	mesh_->SetAttribute(attribute);

	if (mesh_->GetSubMeshCount() == 0) {
		mesh_->AddSubMesh(NewSubMesh());
	}

	TriangleBias bias{ b.indexes.size(), 0, 0 };
	mesh_->GetSubMesh(0)->SetTriangleBias(bias);

	b.material->SetColor(BuiltinProperties::MainColor, b.color);
	Graphics::instance()->Draw(mesh_, b.material);

	Graphics::instance()->SetShadingMode(oldShadingMode);
}
