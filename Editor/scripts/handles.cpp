#include "handles.h"

#include "input.h"
#include "world.h"
#include "screen.h"
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
#define AXIS_ROTATE_SPEED		(0.25f)
#define ARCBALL_ROTATE_SPEED	(0.1f)
#define SCALE_SPEED				(0.02f)
#define GetMaterial0(go)		(go->GetComponent<MeshRenderer>()->GetMaterial(0))

Mesh Handles::s_gizmoMeshes[HandlesMode::size()];
Color Handles::s_gizmoColors[HandlesMode::size()] = {
	Color(0.82f, 0.82f, 0.82f, 0.8f), Color(0.2f, 0.2f, 0.2f, 0.2f), Color(0.82f, 0.82f, 0.82f, 0.8f)
};

Mesh Handles::s_handleMeshes[HandlesMode::size()];
Material Handles::s_materials[Handles::AxisCount + 1];

void Handles::Awake() {
	mode_ = (HandlesMode)-1;
	SetHideFlags(HideFlags::HideInInspector);

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
	handles_->SetLayer(LayerManager::IgnorePick);
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
		SetHandlesMesh(s_handleMeshes[mode_], s_gizmoMeshes[mode_], s_gizmoColors[mode_]);

		if (mode_ != HandlesMode::Rotate) {
			handles_->GetTransform()->SetRotation(glm::quat());
		}

		handler_ = (mode_ == HandlesMode::Move)
			? &Handles::MoveHandles
			: (mode_ == HandlesMode::Rotate) ? &Handles::RotateHandles : &Handles::ScaleHandles;
	}
}

void Handles::SetHandlesMesh(Mesh handle, Mesh gizmo, const Color& color) {
	int i = 0;
	for (Transform transform : handles_->GetTransform()->GetChildren()) {
		Mesh mesh = transform->GetGameObject()->GetComponent<MeshFilter>()->GetMesh();
		Mesh storage = (i < AxisCount) ? handle : gizmo;

		mesh->ShareStorage(storage);
		
		TriangleBias bias{ storage->GetIndexCount() };
		mesh->GetSubMesh(0)->SetTriangleBias(bias);

		if (i == AxisCount) {
			GetMaterial0(transform->GetGameObject())->SetColor(BuiltinProperties::MainColor, color);
		}

		++i;
	}
}

bool Handles::RaycastUnderCursor(RaycastHit& hitInfo) {
	glm::vec3 src = CameraUtility::GetMain()->GetTransform()->GetPosition();
	glm::vec3 dest = CameraUtility::GetMain()->ScreenToWorldPoint(glm::vec3(Input::GetMousePosition(), 1));

	return Physics::Raycast(Ray(src, dest - src), 1000, LayerManager::IgnorePick, &hitInfo);
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

			if (!Math::Approximately(axis_, glm::vec3(1))) {
				GetMaterial0(current_)->SetColor(BuiltinProperties::MainColor, Color(0.8f, 0.8f, 0.8f, 0.4f));
			}
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

		go->SetLayer(LayerManager::IgnorePick);

		Mesh mesh = new IMesh();

		mesh->AddSubMesh(new ISubMesh);

		go->AddComponent<MeshFilter>()->SetMesh(mesh);
		go->AddComponent<MeshRenderer>()->AddMaterial(s_materials[i]);

		go->GetTransform()->SetParent(handles_->GetTransform());
		go->GetTransform()->SetEulerAngles(eulers[i]);

		go->GetComponent<Rigidbody>()->SetOccluderEnabled(false);
	}
}

void Handles::MoveHandles(const glm::vec3& axis, const glm::ivec2& mousePos, const glm::ivec2& oldPos) {
	glm::vec3 dir = Project(axis, mousePos - oldPos);
	GetTransform()->SetPosition(GetTransform()->GetPosition() + dir * MOVE_SPEED);
}

glm::vec3 Handles::GetArcballVector(const glm::ivec2& pos, const glm::ivec2& screenSize) {
	glm::vec3 p(2.f * glm::vec2(pos) / glm::vec2(screenSize) - 1.f, 0);

	float squared = glm::dot(p, p);
	if (squared <= 1.f) {
		p.z = Math::Sqrt(1.f - squared);
	}
	else {
		p = glm::normalize(p);
	}

	return p;
}

glm::vec4 Handles::CalculateArcballRotation(const glm::ivec2& mousePos, const glm::ivec2& oldPos, const glm::ivec2& screenSize) {
	glm::vec3 va = GetArcballVector(oldPos, screenSize);
	glm::vec3 vb = GetArcballVector(mousePos, screenSize);
	float angle = Math::ACos(Math::Min(1.0f, glm::dot(va, vb)));

	return glm::vec4(glm::cross(va, vb), angle);
}

void Handles::RotateHandles(const glm::vec3& axis, const glm::ivec2& mousePos, const glm::ivec2& oldPos) {
	glm::quat q;
	if (!Math::Approximately(axis, glm::vec3(1))) {
		glm::vec3 dir = Project(tangent_, mousePos - oldPos);
		float speed = Math::Radians(glm::length(dir) * Math::Sign(glm::dot(dir, tangent_)) * AXIS_ROTATE_SPEED);
		q = glm::rotate(GetTransform()->GetRotation(), -speed, handles_->GetTransform()->GetRotation() * axis);
	}
	else {
		glm::vec4 param = CalculateArcballRotation(mousePos, oldPos, glm::ivec2(Screen::GetWidth(), Screen::GetHeight()));
		glm::mat3 mv = glm::mat3(CameraUtility::GetMain()->GetTransform()->GetWorldToLocalMatrix()) * glm::mat3(GetTransform()->GetLocalToWorldMatrix());
		glm::vec3 axisModelSpace = glm::inverse(mv) * param.xyz;
		q = glm::rotate(GetTransform()->GetRotation(), Math::Degrees(param.w) * ARCBALL_ROTATE_SPEED, axisModelSpace);
	}

	GetTransform()->SetRotation(q);
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

	Geometries::Cuboid(attribute.positions, attribute.indexes, glm::vec3(0), glm::vec3(0.5f));
	gizmo->SetAttribute(attribute);
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

	Geometries::Cuboid(attribute.positions, attribute.indexes, glm::vec3(0), glm::vec3(0.5f));
	gizmo->SetAttribute(attribute);
}
