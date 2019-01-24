#include "handles.h"

#include <glm/gtc/matrix_transform.hpp>

#include "camera.h"
#include "resources.h"
#include "geometryutility.h"
#include "builtinproperties.h"

SUEDE_DEFINE_COMPONENT(Handles, IBehaviour)

Mesh Handles::mesh_;
Material Handles::materials_[3];

void Handles::Awake() {
	if (!mesh_) { Initialize(); }
}

#include "graphics.h"
void Handles::OnPostRender() {
	float distance = glm::length(CameraUtility::GetMain()->GetTransform()->GetPosition() - GetTransform()->GetPosition());
	glm::vec3 scale(distance / 30.f);

	if (!Math::Approximately(scale, scale_)) {
		for (Material material : materials_) {
			material->SetMatrix4("localToWorldMatrix", glm::trs(glm::vec3(0), glm::quat(), scale));
		}

		scale_ = scale;
	}

	Graphics::Draw(mesh_, materials_, SUEDE_COUNTOF(materials_));
}

void Handles::Initialize() {
	InitializeMesh();
	InitializeMaterial();
}

void Handles::InitializeMesh() {
	MeshAttribute attribute;
	CalculateHandlesGeometry(attribute.positions, attribute.indexes);

	mesh_ = new IMesh();
	mesh_->SetAttribute(attribute);

	uint ni = attribute.indexes.size();
	for (int i = 0; i < SUEDE_COUNTOF(materials_); ++i) {
		SubMesh subMesh = new ISubMesh;
		mesh_->AddSubMesh(subMesh);

		TriangleBias bias{ ni / 3, ni / 3 * i };
		subMesh->SetTriangleBias(bias);
	}
}

void Handles::InitializeMaterial() {
	for (int i = 0; i < SUEDE_COUNTOF(materials_); ++i) {
		materials_[i] = new IMaterial();
		materials_[i]->SetShader(Resources::FindShader("handles"));
	}

	materials_[0]->SetColor(BuiltinProperties::MainColor, Color::red);
	materials_[1]->SetColor(BuiltinProperties::MainColor, Color::green);
	materials_[2]->SetColor(BuiltinProperties::MainColor, Color::blue);
}

void Handles::CalculateHandlesGeometry(std::vector<glm::vec3>& points, std::vector<uint>& indexes) {
	const float kConeLength = 1.2f;
	const float kArrowLength = 7.f;

	glm::vec3 dir(1, 0, 0);

	std::vector<glm::vec3> cylinderPoints, conePoints;
	std::vector<uint> cylinderIndexes, coneIndexes;

	GeometryUtility::GetConeCoordinates(conePoints, coneIndexes, dir * (kArrowLength - kConeLength), dir * kArrowLength, 0.2f, 36);
	GeometryUtility::GetCylinderCoordinates(cylinderPoints, cylinderIndexes, glm::vec3(0), dir * (kArrowLength - kConeLength), 0.05f, 36);

	std::transform(cylinderIndexes.begin(), cylinderIndexes.end(), cylinderIndexes.begin(), [&](uint x) { return x + (uint)conePoints.size(); });

	int np = conePoints.size() + cylinderPoints.size();
	int ni = coneIndexes.size() + cylinderIndexes.size();

	points.resize(np * 3);
	indexes.resize(ni * 3);

	std::copy(conePoints.begin(), conePoints.end(), points.begin());
	std::copy(cylinderPoints.begin(), cylinderPoints.end(), points.begin() + conePoints.size());

	std::copy(coneIndexes.begin(), coneIndexes.end(), indexes.begin());
	std::copy(cylinderIndexes.begin(), cylinderIndexes.end(), indexes.begin() + coneIndexes.size());
	
	glm::quat ry = glm::angleAxis(Math::PiOver2, glm::vec3(0, 0, 1));
	glm::quat rz = glm::angleAxis(-Math::PiOver2, glm::vec3(0, 1, 0));
	for (int i = 0; i < np; ++i) {
		points[np + i] = (ry * points[i]);
		points[np * 2 + i] = (rz * points[i]);
	}

	for (int i = 0; i < ni; ++i) {
		indexes[ni + i] = indexes[i] + np;
		indexes[ni * 2 + i] = indexes[i] + np * 2;
	}
}
