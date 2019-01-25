#include "handles.h"

#include <glm/gtc/matrix_transform.hpp>

#include "time2.h"
#include "input.h"
#include "world.h"
#include "camera.h"
#include "physics.h"
#include "resources.h"
#include "gameobject.h"
#include "layermanager.h"
#include "geometryutility.h"
#include "builtinproperties.h"

SUEDE_DEFINE_COMPONENT(Handles, IBehaviour)

#define N_AXISES				3
#define RAYCAST_INTERVAL		0.1f
#define CONE_LENGTH				1.2f
#define ARROW_LENGTH			7.f
#define GetMaterial0(go)		go->GetComponent<MeshRenderer>()->GetMaterial(0)

GameObject Handles::handles_;

void Handles::Awake() {
	time_ = RAYCAST_INTERVAL;
	if (!handles_) { Initialize(); }
	handles_->GetTransform()->SetParent(World::GetRootTransform());
}

void Handles::Update() {
	if ((time_ -= Time::GetDeltaTime()) < 0) {
		time_ = RAYCAST_INTERVAL;
		UpdateCurrentAxis();
	}

	if (Input::GetMouseButtonDown(0) && current_) {
		pos_ = Input::GetMousePosition();
	}

	if (Input::GetMouseButton(0) && current_) {
		MoveHandles(axis_, Input::GetMousePosition(), pos_);
	}
}

void Handles::OnPreRender() {
	float distance = glm::length(CameraUtility::GetMain()->GetTransform()->GetPosition() - GetTransform()->GetPosition());
	handles_->GetTransform()->SetLocalScale(glm::vec3(distance / 30.f));
}

void Handles::Initialize() {
	handles_ = new IGameObject();
	handles_->SetName("_SuedeHandles");
	handles_->SetLayer(LayerManager::IgnoreRaycast);

	Material materials[N_AXISES];
	InitializeMaterials(materials);

	MeshAttribute attribute;
	CalculateHandlesGeometry(attribute.positions, attribute.indexes);

	Mesh storage = new IMesh();
	storage->SetAttribute(attribute);
	SetupAxises(storage, materials);
}

void Handles::UpdateCurrentAxis() {
	RaycastHit hitInfo;
	glm::vec3 src = CameraUtility::GetMain()->GetTransform()->GetPosition();
	glm::vec3 dest = CameraUtility::GetMain()->ScreenToWorldPoint(glm::vec3(Input::GetMousePosition(), 1));

	Physics::Raycast(Ray(src, dest - src), 1000, LayerManager::IgnoreRaycast, &hitInfo);
	if (current_ != hitInfo.gameObject) {
		if (current_) { GetMaterial0(current_)->SetColor(BuiltinProperties::MainColor, color_); }
		current_ = hitInfo.gameObject;

		if (current_ && !Math::Approximately(axis_ = FindAxis(current_), glm::vec3(0))) {
			color_ = GetMaterial0(current_)->GetColor(BuiltinProperties::MainColor);
			GetMaterial0(current_)->SetColor(BuiltinProperties::MainColor, Color::yellow);
		}
	}
}

glm::vec3 Handles::FindAxis(GameObject current) {
	Transform ht = handles_->GetTransform(), ct = current->GetTransform();
	if (ht->GetChildAt(0) == ct) { return glm::vec3(1, 0, 0); }
	if (ht->GetChildAt(1) == ct) { return glm::vec3(0, 1, 0); }
	if (ht->GetChildAt(2) == ct) { return glm::vec3(0, 0, 1); }
	return glm::vec3(0);
}

void Handles::SetupAxises(Mesh storage, Material * materials) {
	const char* names[] = { "X", "Y", "Z" };
	uint pointCount = storage->GetVertexCount();
	uint indexCount = storage->GetIndexCount();

	for (int i = 0; i < N_AXISES; ++i) {
		GameObject go = new IGameObject();
		go->SetName(names[i]);
		go->SetLayer(LayerManager::IgnoreRaycast);

		Mesh mesh = new IMesh();
		mesh->ShareStorage(storage);

		SubMesh subMesh = new ISubMesh;
		mesh->AddSubMesh(subMesh);

		TriangleBias bias{ indexCount, 0, pointCount / 3 * i };
		subMesh->SetTriangleBias(bias);

		go->AddComponent<MeshFilter>()->SetMesh(mesh);
		go->AddComponent<MeshRenderer>()->AddMaterial(materials[i]);

		go->GetTransform()->SetParent(handles_->GetTransform());
	}
}

void Handles::MoveHandles(const glm::vec3& axis, const glm::ivec2& mousePos, glm::ivec2& oldPos) {
	if (mousePos != oldPos) {
		glm::vec3 dir = axis * glm::dot(axis, glm::vec3(mousePos - oldPos, 0));
		GetTransform()->SetPosition(GetTransform()->GetPosition() + dir);

		oldPos = mousePos;
	}
}

void Handles::InitializeMaterials(Material* materials) {
	Color colors[] = { Color::red, Color::green, Color::blue };

	for (int i = 0; i < N_AXISES; ++i) {
		materials[i] = new IMaterial();
		materials[i]->SetShader(Resources::FindShader("builtin/unlit_colored"));
		materials[i]->SetColor(BuiltinProperties::MainColor, colors[i]);
	}
}

void Handles::CalculateHandlesGeometry(std::vector<glm::vec3>& points, std::vector<uint>& indexes) {
	glm::vec3 dir(1, 0, 0);

	std::vector<glm::vec3> cylinderPoints, conePoints;
	std::vector<uint> cylinderIndexes, coneIndexes;

	GeometryUtility::GetConeCoordinates(conePoints, coneIndexes, dir * (ARROW_LENGTH - CONE_LENGTH), dir * ARROW_LENGTH, 0.2f, 36);
	GeometryUtility::GetCylinderCoordinates(cylinderPoints, cylinderIndexes, glm::vec3(0), dir * (ARROW_LENGTH - CONE_LENGTH), 0.05f, 36);

	std::transform(cylinderIndexes.begin(), cylinderIndexes.end(), cylinderIndexes.begin(), [&](uint x) { return x + (uint)conePoints.size(); });

	int pointCount = conePoints.size() + cylinderPoints.size();
	int indexCount = coneIndexes.size() + cylinderIndexes.size();

	points.resize(pointCount * 3);
	indexes.resize(indexCount);

	std::copy(conePoints.begin(), conePoints.end(), points.begin());
	std::copy(cylinderPoints.begin(), cylinderPoints.end(), points.begin() + conePoints.size());

	std::copy(coneIndexes.begin(), coneIndexes.end(), indexes.begin());
	std::copy(cylinderIndexes.begin(), cylinderIndexes.end(), indexes.begin() + coneIndexes.size());
	
	glm::quat ry = glm::angleAxis(Math::PiOver2, glm::vec3(0, 0, 1));
	glm::quat rz = glm::angleAxis(-Math::PiOver2, glm::vec3(0, 1, 0));
	for (int i = 0; i < pointCount; ++i) {
		points[pointCount + i] = (ry * points[i]);
		points[pointCount * 2 + i] = (rz * points[i]);
	}
}
