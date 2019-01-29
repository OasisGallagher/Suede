#include "handles.h"

#include <glm/gtc/matrix_transform.hpp>

#include "time2.h"
#include "input.h"
#include "world.h"
#include "camera.h"
#include "physics.h"
#include "resources.h"
#include "geometries.h"
#include "gameobject.h"
#include "layermanager.h"
#include "builtinproperties.h"

SUEDE_DEFINE_COMPONENT(Handles, IBehaviour)

#define N_AXISES				(3)
#define RESOLUTION				(27)
#define CONE_LENGTH				(1.2f)
#define ARROW_LENGTH			(7.f)
#define CUBOID_SIZE				(0.4f)
#define MOVE_SPEED				(0.02f)
#define ROTATE_SPEED			(0.08f)
#define SCALE_SPEED				(0.02f)
#define GetMaterial0(go)		(go->GetComponent<MeshRenderer>()->GetMaterial(0))

GameObject Handles::handles_;

void Handles::Awake() {
	mode_ = (HandlesMode)-1;
	if (!handles_) { Initialize(); }
	handles_->GetTransform()->SetParent(World::GetRootTransform());
}

void Handles::Update() {
	if (Input::GetKeyUp(KeyCode::R)) {
		SetMode(HandlesMode::Rotate);
	}

	if (Input::GetKeyUp(KeyCode::M)) {
		SetMode(HandlesMode::Move);
	}

	if (Input::GetKeyUp(KeyCode::S)) {
		SetMode(HandlesMode::Scale);
	}

	if (!Input::GetMouseButton(0)) {
		UpdateCurrentAxis();
	}

	handles_->GetTransform()->SetPosition(GetTransform()->GetPosition());

	glm::ivec2 currentPos = Input::GetMousePosition();

	if (Input::GetMouseButtonDown(0) && current_) {
		pos_ = currentPos;
	}

	if (Input::GetMouseButton(0) && current_ && currentPos != pos_) {
		(this->*method_)(axis_, currentPos, pos_);
		pos_ = currentPos;
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

	for (int i = 0; i < HandlesMode::size(); ++i) {
		meshes_[i] = new IMesh();
	}

	InitializeMoveHandlesMesh(meshes_[HandlesMode::Move]);
	InitializeRotateHandlesMesh(meshes_[HandlesMode::Rotate]);
	InitializeScaleHandlesMesh(meshes_[HandlesMode::Scale]);

	SetupAxises();
	SetMode(HandlesMode::Move);
}

void Handles::SetMode(HandlesMode value) {
	if (mode_ != value) {
		mode_ = value;
		SetHandlesMesh(meshes_[mode_]);
		method_ = (mode_ == HandlesMode::Move)
			? &Handles::MoveHandles
			: (mode_ == HandlesMode::Rotate) ? &Handles::RotateHandles : &Handles::ScaleHandles;
	}
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
			GetMaterial0(current_)->SetColor(BuiltinProperties::MainColor, Color(128 / 255.f, 128 / 255.f, 0));
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

glm::vec3 Handles::Project(const glm::vec3& axis, glm::ivec2 delta) {
	Transform camera = CameraUtility::GetMain()->GetTransform();
	glm::vec3 axisCameraSpace = (camera->GetWorldToLocalMatrix() * glm::vec4(axis, 0)).xyz;
	return axis * glm::dot(axisCameraSpace, glm::vec3(delta, 0));
}

void Handles::SetupAxises() {
	Material materials[N_AXISES];
	InitializeMaterials(materials);

	const char* names[] = { "X", "Y", "Z" };
	glm::vec3 eulers[] = { glm::vec3(0, -90, 0), glm::vec3(-90, 0, 0), glm::vec3(0) };

	for (int i = 0; i < N_AXISES; ++i) {
		GameObject go = new IGameObject();
		go->SetName(names[i]);
		go->SetLayer(LayerManager::IgnoreRaycast);

		Mesh mesh = new IMesh();

		SubMesh subMesh = new ISubMesh;
		mesh->AddSubMesh(subMesh);

		go->AddComponent<MeshFilter>()->SetMesh(mesh);
		go->AddComponent<MeshRenderer>()->AddMaterial(materials[i]);

		go->GetTransform()->SetParent(handles_->GetTransform());
		go->GetTransform()->SetEulerAngles(eulers[i]);
	}
}

void Handles::SetHandlesMesh(Mesh storage) {
	for (Transform transform : handles_->GetTransform()->GetChildren()) {
		Mesh mesh = transform->GetGameObject()->GetComponent<MeshFilter>()->GetMesh();
		mesh->ShareStorage(storage);
		
		TriangleBias bias{ storage->GetIndexCount() };
		mesh->GetSubMesh(0)->SetTriangleBias(bias);
	}
}

void Handles::MoveHandles(const glm::vec3& axis, const glm::ivec2& mousePos, const glm::ivec2& oldPos) {
	glm::vec3 dir = Project(axis, mousePos - oldPos);
	GetTransform()->SetPosition(GetTransform()->GetPosition() + dir * MOVE_SPEED);
}

void Handles::RotateHandles(const glm::vec3& axis, const glm::ivec2& mousePos, const glm::ivec2& oldPos) {
	glm::ivec2 delta = (mousePos - oldPos).yx;
	delta.x = -delta.x;

	glm::vec3 dir = Project(axis, delta);
	GetTransform()->SetEulerAngles(GetTransform()->GetEulerAngles() + dir * ROTATE_SPEED);
}

void Handles::ScaleHandles(const glm::vec3& axis, const glm::ivec2& mousePos, const glm::ivec2& oldPos) {
	glm::vec3 dir = Project(axis, mousePos - oldPos);
	dir.x = -dir.x;
	GetTransform()->SetScale(GetTransform()->GetScale() + dir * SCALE_SPEED);
}

void Handles::InitializeMaterials(Material* materials) {
	Color colors[] = { Color::red, Color::green, Color::blue };

	for (int i = 0; i < N_AXISES; ++i) {
		materials[i] = new IMaterial();
		materials[i]->SetShader(Resources::FindShader("builtin/handles"));
		materials[i]->SetColor(BuiltinProperties::MainColor, colors[i]);
	}
}

void Handles::InitializeMoveHandlesMesh(Mesh mesh) {
	MeshAttribute attribute;

	glm::vec3 dir(0, 0, 1);
	Geometries::Cone(attribute.positions, attribute.indexes, dir * (ARROW_LENGTH - CONE_LENGTH), dir * ARROW_LENGTH, 0.2f, RESOLUTION);
	Geometries::Cylinder(attribute.positions, attribute.indexes, glm::vec3(0), dir * (ARROW_LENGTH - CONE_LENGTH), 0.05f, RESOLUTION);

	mesh->SetAttribute(attribute);
}

void Handles::InitializeRotateHandlesMesh(Mesh mesh) {
	MeshAttribute attribute;
	Geometries::Circle(attribute.positions, attribute.indexes, glm::vec3(0), 5, 0.12f, glm::vec3(0, 0, 1), RESOLUTION);

	mesh->SetAttribute(attribute);
}

void Handles::InitializeScaleHandlesMesh(Mesh mesh) {
	MeshAttribute attribute;

	glm::vec3 dir(0, 0, 1);
	Geometries::Cuboid(attribute.positions, attribute.indexes, dir * (ARROW_LENGTH - CUBOID_SIZE / 2), glm::vec3(CUBOID_SIZE));
	Geometries::Cylinder(attribute.positions, attribute.indexes, glm::vec3(0), dir * (ARROW_LENGTH - CUBOID_SIZE), 0.05f, RESOLUTION);

	mesh->SetAttribute(attribute);
}
