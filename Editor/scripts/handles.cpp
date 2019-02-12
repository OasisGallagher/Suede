#include "handles.h"

#include "input.h"
#include "world.h"
#include "rigidbody.h"
#include "resources.h"
#include "geometries.h"
#include "layermanager.h"
#include "builtinproperties.h"

SUEDE_DEFINE_COMPONENT(Handles, IBehaviour)

#define CONE_LENGTH				(1.2f)
#define ARROW_LENGTH			(7.f)
#define CUBOID_SIZE				(0.4f)
#define MOVE_SPEED				(0.02f)
#define ROTATE_SPEED			(0.08f)
#define SCALE_SPEED				(0.02f)
#define GetMaterial0(go)		(go->GetComponent<MeshRenderer>()->GetMaterial(0))

Mesh Handles::s_handleMeshes[Handles::AxisCount];
Mesh Handles::s_gizmoMeshes[Handles::AxisCount];
Material Handles::s_materials[Handles::AxisCount + 1];

void Handles::Awake() {
	mode_ = (HandlesMode)-1;
	if (!s_handleMeshes[0]) {
		InitializeMeshes();
		InitializeMaterials();
	}

	Initialize();
	handles_->GetTransform()->SetParent(World::GetRootTransform());
}

void Handles::OnDestroy() {
	World::DestroyObject(handles_);
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

	if (mode_ == HandlesMode::Rotate) {
		handles_->GetTransform()->SetRotation(GetTransform()->GetRotation());
	}

	glm::ivec2 currentPos = Input::GetMousePosition();

	if (Input::GetMouseButtonDown(0) && current_) {
		screenPos_ = currentPos;
		Transform transform = handles_->GetTransform();
		tangent_ = glm::normalize(glm::cross(collisionPos_ - transform->GetPosition(), transform->GetRotation() * axis_));
	}

	if (Input::GetMouseButton(0) && current_ && currentPos != screenPos_) {
		(this->*handler_)(axis_, currentPos, screenPos_);
		screenPos_ = currentPos;
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
	handles_->SetHideFlags(HideFlags::HideInHierarchy);

	SetupAxises();
	SetMode(HandlesMode::Rotate);
}

void Handles::InitializeMeshes() {
	for (int i = 0; i < HandlesMode::size(); ++i) {
		s_handleMeshes[i] = new IMesh();
		s_gizmoMeshes[i] = new IMesh();
	}

	InitializeMoveHandlesMesh(s_handleMeshes[HandlesMode::Move], s_gizmoMeshes[HandlesMode::Move]);
	InitializeRotateHandlesMesh(s_handleMeshes[HandlesMode::Rotate], s_gizmoMeshes[HandlesMode::Rotate]);
	InitializeScaleHandlesMesh(s_handleMeshes[HandlesMode::Scale], s_gizmoMeshes[HandlesMode::Scale]);
}

void Handles::InitializeMaterials() {
	Color colors[] = { Color::red, Color::green, Color::blue, Color::clear };

	for (int i = 0; i < SUEDE_COUNTOF(s_materials); ++i) {
		s_materials[i] = new IMaterial();
		s_materials[i]->SetShader(Resources::FindShader("builtin/handles"));
		s_materials[i]->SetColor(BuiltinProperties::MainColor, colors[i]);
	}
}

void Handles::SetMode(HandlesMode value) {
	if (mode_ != value) {
		mode_ = value;
		SetHandlesMesh(s_handleMeshes[mode_], s_gizmoMeshes[mode_]);

		if (mode_ != HandlesMode::Rotate) {
			handles_->GetTransform()->SetRotation(glm::quat());
		}

		handler_ = (mode_ == HandlesMode::Move)
			? &Handles::MoveHandles
			: (mode_ == HandlesMode::Rotate) ? &Handles::RotateHandles : &Handles::ScaleHandles;
	}
}

void Handles::SetHandlesMesh(Mesh handle, Mesh gizmo) {
	int i = 0;
	for (Transform transform : handles_->GetTransform()->GetChildren()) {
		Mesh mesh = transform->GetGameObject()->GetComponent<MeshFilter>()->GetMesh();
		Mesh storage = (i++ < AxisCount) ? handle : gizmo;

		mesh->ShareStorage(storage);
		
		TriangleBias bias{ storage->GetIndexCount() };
		mesh->GetSubMesh(0)->SetTriangleBias(bias);
	}
}

bool Handles::RaycastUnderCursor(RaycastHit& hitInfo) {
	glm::vec3 src = CameraUtility::GetMain()->GetTransform()->GetPosition();
	glm::vec3 dest = CameraUtility::GetMain()->ScreenToWorldPoint(glm::vec3(Input::GetMousePosition(), 1));

	return Physics::Raycast(Ray(src, dest - src), 1000, LayerManager::IgnoreRaycast, &hitInfo);
}

Color Handles::GetActiveAxisColor(const glm::vec3& axis) {
	return Math::Approximately(axis, glm::vec3(1)) ? Color(0.5f, 0.5f, 0.5f, 0.4f) : Color(128 / 255.f, 128 / 255.f, 0);
}

void Handles::UpdateCurrentAxis() {
	RaycastHit hitInfo;
	RaycastUnderCursor(hitInfo);

	if (current_ != hitInfo.gameObject) {
		if (current_) { GetMaterial0(current_)->SetColor(BuiltinProperties::MainColor, oldColor_); }
		current_ = hitInfo.gameObject;

		if (current_ && !Math::Approximately(axis_ = FindAxis(current_), glm::vec3(0))) {
			collisionPos_ = hitInfo.point;
			oldColor_ = GetMaterial0(current_)->GetColor(BuiltinProperties::MainColor);

			GetMaterial0(current_)->SetColor(BuiltinProperties::MainColor, GetActiveAxisColor(axis_));
			Debug::LogWarning("selected %s", current_ ? current_->GetName().c_str() : "null");
		}
	}
}

glm::vec3 Handles::FindAxis(GameObject current) {
	Transform ht = handles_->GetTransform(), ct = current->GetTransform();
	if (ht->GetChildAt(0) == ct) { return glm::vec3(1, 0, 0); }
	if (ht->GetChildAt(1) == ct) { return glm::vec3(0, 1, 0); }
	if (ht->GetChildAt(2) == ct) { return glm::vec3(0, 0, 1); }
	if (ht->GetChildAt(3) == ct) { return glm::vec3(1); }
	return glm::vec3(0);
}

glm::vec3 Handles::Project(const glm::vec3& axis, const glm::ivec2& delta) {
	Transform camera = CameraUtility::GetMain()->GetTransform();
	glm::vec3 axisCameraSpace = (camera->GetWorldToLocalMatrix() * glm::vec4(axis, 0)).xyz;
	return axis * glm::dot(axisCameraSpace, glm::vec3(delta, 0));
}

void Handles::SetupAxises() {
	const char* names[] = { "X", "Y", "Z", "Gizmo" };
	glm::vec3 eulers[] = { glm::vec3(0, -90, 0), glm::vec3(-90, 0, 0), glm::vec3(0), glm::vec3(0) };

	for (int i = 0; i < AxisCount + 1; ++i) {
		GameObject go = new IGameObject();
		go->SetName(names[i]);
		go->SetLayer(LayerManager::IgnoreRaycast);

		Mesh mesh = new IMesh();

		SubMesh subMesh = new ISubMesh;
		mesh->AddSubMesh(subMesh);

		go->AddComponent<MeshFilter>()->SetMesh(mesh);
		go->AddComponent<MeshRenderer>()->AddMaterial(s_materials[i]);

		go->GetTransform()->SetParent(handles_->GetTransform());
		go->GetTransform()->SetEulerAngles(eulers[i]);
	}
}

void Handles::MoveHandles(const glm::vec3& axis, const glm::ivec2& mousePos, const glm::ivec2& oldPos) {
	glm::vec3 dir = Project(axis, mousePos - oldPos);
	GetTransform()->SetPosition(GetTransform()->GetPosition() + dir * MOVE_SPEED);
}

void Handles::RotateHandles(const glm::vec3& axis, const glm::ivec2& mousePos, const glm::ivec2& oldPos) {
	glm::ivec2 delta = (mousePos - oldPos);

	glm::vec3 dir = Project(tangent_, delta);
	float speed = Math::Radians(glm::length(dir) * Math::Sign(glm::dot(dir, tangent_)) * ROTATE_SPEED);
	GetTransform()->SetRotation(glm::angleAxis(speed, handles_->GetTransform()->GetRotation() * axis) * GetTransform()->GetRotation());
}

void Handles::ScaleHandles(const glm::vec3& axis, const glm::ivec2& mousePos, const glm::ivec2& oldPos) {
	glm::vec3 dir = Project(axis, mousePos - oldPos);
	dir.x = -dir.x;
	GetTransform()->SetScale(GetTransform()->GetScale() + dir * SCALE_SPEED);
}

void Handles::InitializeMoveHandlesMesh(Mesh handle, Mesh gizmo) {
	MeshAttribute attribute;

	glm::vec3 dir(0, 0, 1);
	Geometries::Cone(attribute.positions, attribute.indexes, dir * (ARROW_LENGTH - CONE_LENGTH), dir * ARROW_LENGTH, 0.2f, Resolution);
	Geometries::Cylinder(attribute.positions, attribute.indexes, glm::vec3(0), dir * (ARROW_LENGTH - CONE_LENGTH), 0.05f, Resolution);

	handle->SetAttribute(attribute);

	attribute.positions.clear();
	attribute.indexes.clear();
}

void Handles::InitializeRotateHandlesMesh(Mesh handle, Mesh gizmo) {
	MeshAttribute attribute;
	Geometries::Circle(attribute.positions, attribute.indexes, glm::vec3(0), 5, 0.12f, glm::vec3(0, 0, 1), Resolution);
	handle->SetAttribute(attribute);

	attribute.positions.clear();
	attribute.indexes.clear();

	Geometries::Sphere(attribute.positions, attribute.indexes, glm::vec3(0), 4.7f, glm::ivec2(Resolution));
	gizmo->SetAttribute(attribute);
}

void Handles::InitializeScaleHandlesMesh(Mesh handle, Mesh gizmo) {
	MeshAttribute attribute;

	glm::vec3 dir(0, 0, 1);
	Geometries::Cuboid(attribute.positions, attribute.indexes, dir * (ARROW_LENGTH - CUBOID_SIZE / 2), glm::vec3(CUBOID_SIZE));
	Geometries::Cylinder(attribute.positions, attribute.indexes, glm::vec3(0), dir * (ARROW_LENGTH - CUBOID_SIZE), 0.05f, Resolution);

	handle->SetAttribute(attribute);

	attribute.positions.clear();
	attribute.indexes.clear();
}
