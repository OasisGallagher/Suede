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

static std::vector<uint> pointIndexes;
static std::vector<glm::vec3> linePoints;

static void Initialize() {
	Shader shader = NewShader();
	if (!shader->Load("buildin/shaders/gizmos")) {
		return;
	}

	mesh = NewMesh();
	material = NewMaterial();
	material->SetShader(shader);
	material->SetColor4(Variables::mainColor, glm::vec4(0, 1, 0, 1));
}

glm::vec3 Gizmos::GetColor() {
	return color;
}

void Gizmos::SetColor(const glm::vec3& value) {
	color = value;
}

void Gizmos::DrawLines(const glm::vec3* points, uint npoints) {
	uint base = linePoints.size();
	linePoints.insert(linePoints.end(), points, points + npoints);
	for (uint i = 0; i < npoints; ++i) {
		pointIndexes.push_back(base + i);
	}
}

void Gizmos::DrawLines(const glm::vec3* points, uint npoints, uint* indexes, uint nindexes) {
	uint base = linePoints.size();
	linePoints.insert(linePoints.end(), points, points + npoints);
	for (uint i = 0; i < nindexes; ++i) {
		pointIndexes.push_back(base + indexes[i]);
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
	if (linePoints.empty()) {
		return;
	}

	if (!material) { Initialize(); }

	MeshAttribute attribute;
	attribute.topology = MeshTopologyLines;

	attribute.positions = linePoints;
	attribute.indexes = pointIndexes;

	mesh->SetAttribute(attribute);

	if (mesh->GetSubMeshCount() != 0) {
		mesh->RemoveSubMesh(0);
	}

	SubMesh subMesh = NewSubMesh();
	TriangleBias bias{ pointIndexes.size(), 0, 0 };
	subMesh->SetTriangleBias(bias);
	mesh->AddSubMesh(subMesh);

	linePoints.clear();

	Graphics::Draw(mesh, material);
}
