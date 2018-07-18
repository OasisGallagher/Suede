#include <vector>

#include "mesh.h"
#include "gizmos.h"
#include "material.h"
#include "graphics.h"
#include "resources.h"
#include "variables.h"
#include "tools/math2.h"
#include "geometryutility.h"

Gizmos::Gizmos() : color_(0, 1, 0) {
	mesh_ = NewMesh();

	material_ = NewMaterial();
	material_->SetShader(Resources::get()->FindShader("builtin/gizmos"));
	material_->SetColor4(Variables::MainColor, glm::vec4(color_, 1));
}

Gizmos::Batch& Gizmos::GetBatch() {
	if (batches_.empty() || batches_.back().color != color_) {
		Batch b = { color_ };
		batches_.push_back(b);
	}

	return batches_.back();
}

glm::vec3 Gizmos::GetColor() {
	return color_;
}

void Gizmos::SetColor(const glm::vec3& value) {
	color_ = value;
}

void Gizmos::DrawLines(const glm::vec3* points, uint npoints) {
	Batch& b = GetBatch();

	uint base = b.points.size();
	b.points.insert(b.points.end(), points, points + npoints);
	for (uint i = 0; i < npoints; ++i) {
		b.indexes.push_back(base + i);
	}
}

void Gizmos::DrawLines(const glm::vec3* points, uint npoints, uint* indexes, uint nindexes) {
	Batch& b = GetBatch();

	uint base = b.points.size();
	b.points.insert(b.points.end(), points, points + npoints);

	for (uint i = 0; i < nindexes; ++i) {
		b.indexes.push_back(base + indexes[i]);
	}
}

void Gizmos::DrawCuboid(const glm::vec3& center, const glm::vec3& size) {
	std::vector<glm::vec3> points;
	GeometryUtility::GetCuboidCoordinates(points, center, size);

	uint indexes[] = {
		0, 1, 1, 2, 2, 3, 3, 0,
		4, 5, 5, 6, 6, 7, 7, 4,
		0, 4, 1, 5, 2, 6, 3, 7,
	};

	DrawLines(&points[0], points.size(), indexes, CountOf(indexes));
}

void Gizmos::Flush() {
	for (uint i = 0; i < batches_.size(); ++i) {
		const Batch& b = batches_[i];
		MeshAttribute attribute;
		attribute.topology = MeshTopology::Lines;

		attribute.positions = b.points;
		attribute.indexes = b.indexes;

		mesh_->SetAttribute(attribute);

		if (mesh_->GetSubMeshCount() == 0) {
			mesh_->AddSubMesh(NewSubMesh());
		}

		TriangleBias bias{ b.indexes.size(), 0, 0 };
		mesh_->GetSubMesh(0)->SetTriangleBias(bias);

		material_->SetColor4(Variables::MainColor, glm::vec4(b.color, 1));
		Graphics::Draw(mesh_, material_);
	}

	batches_.clear();
}
