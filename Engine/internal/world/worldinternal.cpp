#include "worldinternal.h"

#include "time2.h"
#include "profiler.h"
#include "statistics.h"
#include "tools/math2.h"
#include "decalcreater.h"
#include "../api/glutils.h"
#include "geometryutility.h"
#include "internal/async/guard.h"
#include "internal/rendering/shadows.h"
#include "internal/codec/gameObjectloader.h"
#include "internal/rendering/matrixbuffer.h"
#include "internal/world/environmentinternal.h"
#include "internal/gameobject/gameobjectinternal.h"
#include "internal/components/transforminternal.h"
#include "internal/rendering/uniformbuffermanager.h"

#include "internal/tools/gizmosinternal.h"
#include "internal/tools/graphicsinternal.h"
#include "internal/tools/resourcesinternal.h"

#define ColorLuminance(color)	(0.299f * color.r + 0.587f * color.g + 0.114f * color.b)

bool WorldInternal::LightComparer::operator()(const Light& lhs, const Light& rhs) const {
	// Directional light > Importance > Luminance.
	LightType lt = lhs->GetType(), rt = rhs->GetType();
	if (lt != rt && (lt == LightType::Directional || rt == LightType::Directional)) {
		return lt == LightType::Directional;
	}

	LightImportance lli = lhs->GetImportance(), rli = rhs->GetImportance();
	if (lli != rli) {
		return lli > rli;
	}

	return ColorLuminance(lhs->GetColor()) > ColorLuminance(rhs->GetColor());
}

bool WorldInternal::CameraComparer::operator() (const Camera& lhs, const Camera& rhs) const {
	return lhs->GetDepth() < rhs->GetDepth();
}

bool WorldInternal::ProjectorComparer::operator() (const Projector& lhs, const Projector& rhs) const {
	return lhs->GetDepth() < rhs->GetDepth();
}

WorldInternal::WorldInternal()
	: importer_(MEMORY_NEW(GameObjectLoaderThreadPool)) {
	Screen::instance()->AddScreenSizeChangedListener(this);
	AddEventListener(this);
}

void WorldInternal::Initialize() {
	GLUtils::Initialize();

	Resources::implement(new ResourcesInternal);
	Resources::instance()->FindShader("builtin/lit_texture");

	Gizmos::implement(new GizmosInternal);
	Graphics::implement(new GraphicsInternal);
	Environment::implement(new EnvironmentInternal);

	UniformBufferManager::instance();
	Shadows::instance();
	MatrixBuffer::instance();

	decalCreater_ = MEMORY_NEW(DecalCreater);

	root_ = Factory::Create<GameObjectInternal>();
	root_->AddComponent<ITransform>();
	root_->SetName("Root");
}

WorldInternal::~WorldInternal() {
}

void WorldInternal::Finalize() {
	for (Camera camera : cameras_) {
		camera->OnBeforeWorldDestroyed();
	}

	MEMORY_DELETE(importer_);
	MEMORY_DELETE(decalCreater_);

	RemoveEventListener(this);
	Screen::instance()->RemoveScreenSizeChangedListener(this);
}

Object WorldInternal::CreateObject(ObjectType type) {
	Object object = Factory::Create(type);
	AddObject(object);
	return object;
}

GameObject WorldInternal::Import(const std::string& path, GameObjectLoadedListener* listener) {
	importer_->SetLoadedListener(listener);
	return importer_->Import(path);
}

bool WorldInternal::ImportTo(GameObject go, const std::string& path, GameObjectLoadedListener* listener) {
	importer_->SetLoadedListener(listener);
	return importer_->ImportTo(go, path);
}

GameObject WorldInternal::GetGameObject(uint id) {
	GameObjectDictionary::iterator ite = gameObjects_.find(id);
	if (ite == gameObjects_.end()) { return nullptr; }
	return ite->second;
}

void WorldInternal::DestroyGameObject(uint id) {
	GameObject go = GetGameObject(id);
	if (go) {
		DestroyGameObject(go);
	}
}

void WorldInternal::DestroyGameObject(GameObject go) {
	DestroyGameObjectRecursively(go->GetTransform());
}

void WorldInternal::DestroyGameObjectRecursively(Transform root) {
	GameObject go = root->GetGameObject();

	Camera camera = go->GetComponent<ICamera>();
	if (camera) { cameras_.erase(camera); }

	Light light = go->GetComponent<ILight>();
	if (light) { lights_.erase(light); }

	Projector projector = go->GetComponent<IProjector>();
	if (projector) { projectors_.erase(projector); }

	RemoveGameObjectFromSequence(go);
	gameObjects_.erase(go->GetInstanceID());
	go->GetTransform()->SetParent(nullptr);

	GameObjectDestroyedEventPointer e = NewWorldEvent<GameObjectDestroyedEventPointer>();
	e->go = go;
	FireEvent(e);

	for(Transform transform : root->GetChildren()) {
		DestroyGameObjectRecursively(transform);
	}
}

std::vector<GameObject> WorldInternal::GetGameObjectsOfComponent(suede_guid guid) {
	std::vector<GameObject> gameObjects;
	if (guid == ICamera::GetComponentGUID()) {
		for (Camera camera : cameras_) {
			gameObjects.push_back(camera->GetGameObject());
		}
	}
	else if (guid == IProjector::GetComponentGUID()) {
		for (Projector projector : projectors_) {
			gameObjects.push_back(projector->GetGameObject());
		}
	}
	else if (guid == ILight::GetComponentGUID()) {
		for (Light light : lights_) {
			gameObjects.push_back(light->GetGameObject());
		}
	}
	else {
		for (GameObjectDictionary::iterator ite = gameObjects_.begin(); ite != gameObjects_.end(); ++ite) {
			if (ite->second->GetComponent(guid)) {
				gameObjects.push_back(ite->second);
			}
		}
	}

	return gameObjects;
}

void WorldInternal::WalkGameObjectHierarchy(WorldGameObjectWalker* walker) {
	ZTHREAD_LOCK_SCOPE(TransformInternal::hierarchyMutex);
	WalkGameObjectHierarchyRecursively(GetRootTransform(), walker);
}

void WorldInternal::AddEventListener(WorldEventListener* listener) {
	if (std::find(listeners_.begin(), listeners_.end(), listener) == listeners_.end()) {
		listeners_.push_back(listener);
	}
}

void WorldInternal::RemoveEventListener(WorldEventListener* listener) {
	EventListenerContainer::iterator pos = std::find(listeners_.begin(), listeners_.end(), listener);
	if (pos != listeners_.end()) {
		listeners_.erase(pos);
	}
}

void WorldInternal::FireEvent(WorldEventBasePointer e) {
	WorldEventType type = e->GetEventType();
	WorldEventCollection& collection = events_[(int)type];

	ZTHREAD_LOCK_SCOPE(eventsMutex_);
	collection.push_back(e);
}

void WorldInternal::FireEventImmediate(WorldEventBasePointer e) {
	for (WorldEventListener* listener : listeners_) {
		listener->OnWorldEvent(e);
	}
}

void WorldInternal::GetDecals(std::vector<Decal>& container) {
	decalCreater_->GetDecals(container);
}

void WorldInternal::UpdateDecals() {
	Camera main = Camera::GetMain();
	if (main) {
		decalCreater_->Update(main, projectors_);
	}
}

void WorldInternal::CullingUpdateGameObjects() {
	for (GameObject go : cullingUpdateSequence_) {
		if (go->GetActive()) {
			go->CullingUpdate();
		}
	}
}

void WorldInternal::RenderingUpdateGameObjects() {
	for (GameObject go : renderingUpdateSequence_) {
		if (go->GetActive()) {
			go->RenderingUpdate();
		}
	}
}

bool WorldInternal::WalkGameObjectHierarchyRecursively(Transform root, WorldGameObjectWalker* walker) {
	for(Transform transform : root->GetChildren()) {
		GameObject child = transform->GetGameObject();
		if (!child) {
			continue;
		}

		WalkCommand command = walker->OnWalkGameObject(child);

		// next sibling.
		if (command == WalkCommand::Next) {
			continue;
		}

		// 
		if (command == WalkCommand::Break) {
			return false;
		}

		if (!WalkGameObjectHierarchyRecursively(child->GetTransform(), walker)) {
			return false;
		}
	}

	return true;
}

void WorldInternal::OnScreenSizeChanged(uint width, uint height) {
}

void WorldInternal::OnWorldEvent(WorldEventBasePointer e) {
	switch (e->GetEventType()) {
		case WorldEventType::CameraDepthChanged:
			cameras_.sort();
			break;
		case WorldEventType::GameObjectParentChanged:
			OnGameObjectParentChanged(suede_static_cast<GameObjectEventPointer>(e)->go);
			break;
		case WorldEventType::GameObjectUpdateStrategyChanged:
			ManageGameObjectUpdateSequence(suede_static_cast<GameObjectEventPointer>(e)->go);
			break;
		case WorldEventType::GameObjectComponentChanged:
			OnGameObjectComponentChanged(suede_static_cast<GameObjectComponentChangedEventPointer>(e));
			break;
	}
}

void WorldInternal::AddObject(Object object) {
	ObjectType type = object->GetObjectType();
	if (type >= ObjectType::GameObject) {
		GameObject go = suede_dynamic_cast<GameObject>(object);
		go->AddComponent<ITransform>();

		GameObjectCreatedEventPointer e = NewWorldEvent<GameObjectCreatedEventPointer>();
		e->go = go;
		FireEvent(e);

		ZTHREAD_LOCK_SCOPE(TransformInternal::hierarchyMutex);
		gameObjects_.insert(std::make_pair(go->GetInstanceID(), go));
	}
}

void WorldInternal::OnGameObjectParentChanged(GameObject go) {
	if (go->GetTransform()->GetParent()) {
		gameObjects_.insert(std::make_pair(go->GetInstanceID(), go));
	}
	else {
		gameObjects_.erase(go->GetInstanceID());
	}
}

void WorldInternal::OnGameObjectComponentChanged(GameObjectComponentChangedEventPointer e) {
	ManageGameObjectUpdateSequence(e->go);
	ManageGameObjectComponents(lights_, e->component, e->added);
	ManageGameObjectComponents(cameras_, e->component, e->added);
	ManageGameObjectComponents(projectors_, e->component, e->added);
}

void WorldInternal::FireEvents() {
	for (uint i = 0; i < (int)WorldEventType::_Count; ++i) {
		for (WorldEventBasePointer pointer : events_[i]) {
			FireEventImmediate(pointer);
		}
	}

	ZTHREAD_LOCK_SCOPE(eventsMutex_);
	for (uint i = 0; i < (int)WorldEventType::_Count; ++i) {
		events_[i].clear();
	}
}

void WorldInternal::UpdateTimeUniformBuffer() {
	static SharedTimeUniformBuffer p;
	p.time.x = Time::instance()->GetRealTimeSinceStartup();
	p.time.y = Time::instance()->GetDeltaTime();
	UniformBufferManager::instance()->Update(SharedTimeUniformBuffer::GetName(), &p, 0, sizeof(p));
}

void WorldInternal::RemoveGameObjectFromSequence(GameObject go) {
	cullingUpdateSequence_.erase(go);
	renderingUpdateSequence_.erase(go);
}

void WorldInternal::ManageGameObjectUpdateSequence(GameObject go) {
	int strategy = go->GetUpdateStrategy();
	if ((strategy & UpdateStrategyCulling) != 0) {
		if (!cullingUpdateSequence_.contains(go)) {
			cullingUpdateSequence_.insert(go);
		}
	}
	else {
		cullingUpdateSequence_.erase(go);
	}

	if ((strategy & UpdateStrategyRendering) != 0) {
		if (!renderingUpdateSequence_.contains(go)) {
			renderingUpdateSequence_.insert(go);
		}
	}
	else {
		renderingUpdateSequence_.erase(go);
	}
}

void WorldInternal::CullingUpdate() {
	CullingUpdateGameObjects();
}

void WorldInternal::RenderingUpdate() {
	uint64 start = Profiler::instance()->GetTimeStamp();
	FireEvents();

	// SUEDE TODO: update decals in rendering thread ?
	UpdateDecals();

	RenderingUpdateGameObjects();

	// SUEDE TODO: CLEAR STENCIL BUFFER.
	//Framebuffer0::Get()->Clear(FramebufferClearMaskColorDepthStencil);

	UpdateTimeUniformBuffer();

	for (Camera camera : cameras_) {
		if (camera->GetEnabled()) {
			camera->Render();
		}
	}

	Statistics::instance()->SetRenderingElapsed(
		Profiler::instance()->TimeStampToSeconds(Profiler::instance()->GetTimeStamp() - start)
	);
}
