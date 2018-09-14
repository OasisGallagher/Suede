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
#include "internal/entities/gameobjectinternal.h"
#include "internal/components/transforminternal.h"
#include "internal/rendering/uniformbuffermanager.h"

#include "internal/tools/gizmosinternal.h"
#include "internal/tools/graphicsinternal.h"
#include "internal/tools/resourcesinternal.h"

bool WorldInternal::LightComparer::operator()(const Light& lhs, const Light& rhs) const {
	// Directional light > Importance > Luminance.
	ObjectType lt = lhs->GetObjectType(), rt = rhs->GetObjectType();
	if (lt != rt && (lt == ObjectType::DirectionalLight || rt == ObjectType::DirectionalLight)) {
		return lt == ObjectType::DirectionalLight;
	}

	LightImportance lli = lhs->GetImportance(), rli = rhs->GetImportance();
	if (lli != rli) {
		return lli > rli;
	}

	return Math::Luminance(lhs->GetColor()) > Math::Luminance(rhs->GetColor());
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
	Resources::instance()->FindShader("builtin/ssao");

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
	GameObjectDictionary::iterator ite = entities_.find(id);
	if (ite == entities_.end()) { return nullptr; }
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

	// SUEDE TODO: tag go with destroyed?
	RemoveGameObjectFromSequence(go);
	entities_.erase(go->GetInstanceID());
	go->GetTransform()->SetParent(nullptr);

	GameObjectDestroyedEventPointer e = NewWorldEvent<GameObjectDestroyedEventPointer>();
	e->go = go;
	FireEvent(e);

	for(Transform transform : root->GetChildren()) {
		DestroyGameObjectRecursively(transform);
	}
}

bool WorldInternal::GetEntities(ObjectType type, std::vector<GameObject>& entities) {
	if (type < ObjectType::GameObject) {
		Debug::LogError("invalid go type");
		return false;
	}

	return CollectEntities(type, entities);
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

void WorldInternal::CullingUpdateEntities() {
	for (GameObject go : cullingUpdateSequence_) {
		if (go->GetActive()) {
			go->CullingUpdate();
		}
	}
}

void WorldInternal::RenderingUpdateEntities() {
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
			AddGameObjectToUpdateSequence(suede_static_cast<GameObjectEventPointer>(e)->go);
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
		entities_.insert(std::make_pair(go->GetInstanceID(), go));
	}
}

bool WorldInternal::CollectEntities(ObjectType type, std::vector<GameObject>& entities) {
	if (type == ObjectType::GameObject) {
		for (GameObjectDictionary::iterator ite = entities_.begin(); ite != entities_.end(); ++ite) {
			entities.push_back(ite->second);
		}
	}
	else if (type == ObjectType::Camera) {
		for (Camera camera : cameras_) {
			entities.push_back(camera->GetGameObject());
		}
	}
	else if (type == ObjectType::Projector) {
		for (Projector projector : projectors_) {
			entities.push_back(projector->GetGameObject());
		}
	}
	else if (type == SUEDE_ALL_LIGHTS) {
		for (Light light : lights_) {
			entities.push_back(light->GetGameObject());
		}
	}
	else {
		Debug::LogError("not implemented");
		// SUEDE TODO: Get entities of type.
//		for (GameObjectDictionary::iterator ite = entities_.begin(); ite != entities_.end(); ++ite) {
//			if (ite->second->GetType() == type) {
//				entities.push_back(ite->second);
//			}
//		}
	}

	return !entities.empty();
}

void WorldInternal::OnGameObjectParentChanged(GameObject go) {
	if (go->GetTransform()->GetParent()) {
		entities_.insert(std::make_pair(go->GetInstanceID(), go));
	}
	else {
		entities_.erase(go->GetInstanceID());
	}
}

void WorldInternal::OnGameObjectComponentChanged(GameObjectComponentChangedEventPointer e) {
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

void WorldInternal::AddGameObjectToUpdateSequence(GameObject go) {
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
	CullingUpdateEntities();
}

void WorldInternal::RenderingUpdate() {
	uint64 start = Profiler::instance()->GetTimeStamp();
	FireEvents();

	// SUEDE TODO: update decals in rendering thread ?
	UpdateDecals();

	RenderingUpdateEntities();

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
