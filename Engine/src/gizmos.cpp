#include <vector>

#include "mesh.h"
#include "gizmos.h"
#include "material.h"
#include "graphics.h"
#include "variables.h"
#include "tools/math2.h"
#include "geometryutility.h"

static Mesh mesh;
static Material material;

static glm::vec3 color;

struct Batch {
	glm::vec3 color;
	std::vector<uint> indexes;
	std::vector<glm::vec3> points;
};

static std::vector<Batch> batches;

static Batch& GetBatch() {
	if (batches.empty() || batches.back().color != color) {
		Batch b = { color };
		batches.push_back(b);
	}

	return batches.back();
}

static void Initialize() {
	Shader shader = NewShader();
	if (!shader->Load("builtin/gizmos")) {
		return;
	}

	mesh = NewMesh();
	color = glm::vec3(0, 1, 0);

	material = NewMaterial();
	material->SetShader(shader);
	material->SetColor4(Variables::mainColor, glm::vec4(color, 1));
}

glm::vec3 Gizmos::GetColor() {
	return color;
}

void Gizmos::SetColor(const glm::vec3& value) {
	color = value;
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
	if (!material) { Initialize(); }
	for (uint i = 0; i < batches.size(); ++i) {
		const Batch& b = batches[i];
		MeshAttribute attribute;
		attribute.topology = MeshTopologyLines;

		attribute.positions = b.points;
		attribute.indexes = b.indexes;

		mesh->SetAttribute(attribute);

		if (mesh->GetSubMeshCount() == 0) {
			mesh->AddSubMesh(NewSubMesh());
		}

		TriangleBias bias{ b.indexes.size(), 0, 0 };
		mesh->GetSubMesh(0)->SetTriangleBias(bias);

		material->SetColor4(Variables::mainColor, glm::vec4(b.color, 1));
		Graphics::Draw(mesh, material);
	}

	batches.clear();
}
