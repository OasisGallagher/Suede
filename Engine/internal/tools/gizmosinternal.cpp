#include "gizmosinternal.h"

#include "graphics.h"
#include "variables.h"
#include "resources.h"
#include "tools/math2.h"
#include "geometryutility.h"

GizmosInternal::GizmosInternal() : color_(0, 1, 0) {
	mesh_ = NewMesh();

	material_ = NewMaterial();
	material_->SetShader(Resources::instance()->FindShader("builtin/gizmos"));
	material_->SetColor4(Variables::MainColor, glm::vec4(color_, 1));
}

GizmosBatch& GizmosInternal::GetBatch(MeshTopology topology) {
	if (batches_.empty() || batches_.back().color != color_) {
		GizmosBatch b = { topology, color_ };
		batches_.push_back(b);
	}

	return batches_.back();
}

glm::vec3 GizmosInternal::GetColor() {
	return color_;
}

void GizmosInternal::SetColor(const glm::vec3& value) {
	color_ = value;
}

void GizmosInternal::DrawLines(const glm::vec3* points, uint npoints) {
	FillBatch(GetBatch(MeshTopology::Lines), points, npoints);
}

void GizmosInternal::DrawLines(const glm::vec3* points, uint npoints, const uint* indexes, uint nindexes) {
	FillBatch(GetBatch(MeshTopology::Lines), points, npoints, nindexes, indexes);
}

void GizmosInternal::DrawLineStripe(const glm::vec3 * points, uint npoints) {
	FillBatch(GetBatch(MeshTopology::LineStripe), points, npoints);
}

void GizmosInternal::DrawLineStripe(const glm::vec3 * points, uint npoints, const uint * indexes, uint nindexes) {
	FillBatch(GetBatch(MeshTopology::LineStripe), points, npoints, nindexes, indexes);
}

// https://stackoverflow.com/questions/7687148/drawing-sphere-in-opengl-without-using-glusphere
void GizmosInternal::DrawSphere(const glm::vec3& center, float radius) {
	std::vector<glm::vec3> points;
	GetSphereCoodrinates(points, radius, center, glm::ivec2(12));
	DrawLines(&points[0], points.size());
}

void GizmosInternal::DrawCuboid(const glm::vec3& center, const glm::vec3& size) {
	std::vector<glm::vec3> points;
	GeometryUtility::GetCuboidCoordinates(points, center, size);

	uint indexes[] = {
		0, 1, 1, 2, 2, 3, 3, 0,
		4, 5, 5, 6, 6, 7, 7, 4,
		0, 4, 1, 5, 2, 6, 3, 7,
	};

	DrawLines(&points[0], points.size(), indexes, CountOf(indexes));
}

void GizmosInternal::Flush() {
	for (GizmosBatch& b : batches_) {
		RenderGizmos(b);
	}

	batches_.clear();
}

void GizmosInternal::FillBatch(GizmosBatch &b, const glm::vec3* points, uint npoints, uint nindexes, const uint* indexes) {
	uint base = b.points.size();
	b.points.insert(b.points.end(), points, points + npoints);

	for (uint i = 0; i < nindexes; ++i) {
		b.indexes.push_back(base + indexes[i]);
	}
}

void GizmosInternal::FillBatch(GizmosBatch &b, const glm::vec3* points, uint npoints) {
	uint base = b.points.size();
	b.points.insert(b.points.end(), points, points + npoints);
	for (uint i = 0; i < npoints; ++i) {
		b.indexes.push_back(base + i);
	}
}

void GizmosInternal::RenderGizmos(const GizmosBatch& b) {
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
	Graphics::instance()->Draw(mesh_, material_);
}

void GizmosInternal::GetSphereCoodrinates(std::vector<glm::vec3>& points, float radius, const glm::vec3& center, const glm::ivec2& resolution) {
	// step size between U-points on the grid
	glm::vec2 step = glm::vec2(Math::Pi() * 2, Math::Pi()) / glm::vec2(resolution);

	for (float i = 0; i < resolution.x; i++) { // U-points
		for (float j = 0; j < resolution.y; j++) { // V-points
			glm::vec2 uv = glm::vec2(i, j) * step;
			float un = ((i + 1) == resolution.x) ? Math::Pi() * 2 : (i + 1) * step.x;
			float vn = ((j + 1) == resolution.y) ? Math::Pi() : (j + 1) * step.y;

			// Find the four points of the grid square by evaluating the parametric urface function.
			glm::vec3 p0 = SphereCoodrinate(uv.x, uv.y, radius) + center;
			glm::vec3 p1 = SphereCoodrinate(uv.x, vn, radius) + center;
			glm::vec3 p2 = SphereCoodrinate(un, uv.y, radius) + center;
			glm::vec3 p3 = SphereCoodrinate(un, vn, radius) + center;

			// NOTE: For spheres, the normal is just the normalized
			// version of each vertex point; this generally won't be the case for
			// other parametric surfaces.
			// Output the first triangle of this grid square
			// 0 -> 2 -> 1
			points.push_back(p0); points.push_back(p2); points.push_back(p2); points.push_back(p1); points.push_back(p1); points.push_back(p0);

			// Output the other triangle of this grid square
			// 3 -> 1 -> 2
			points.push_back(p3); points.push_back(p1); points.push_back(p1); points.push_back(p2); points.push_back(p2); points.push_back(p3);
		}
	}
}
