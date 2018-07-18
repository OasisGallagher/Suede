#include "time2.h"
#include "resources.h"
#include "api/glutils.h"
#include "worldinternal.h"
#include "debug/profiler.h"
#include "geometryutility.h"
#include "internal/async/guard.h"
#include "internal/rendering/shadows.h"
#include "internal/codec/entityloader.h"
#include "internal/base/textureinternal.h"
#include "internal/rendering/matrixbuffer.h"
#include "internal/entities/entityinternal.h"
#include "internal/world/environmentinternal.h"
#include "internal/components/transforminternal.h"
#include "internal/rendering/uniformbuffermanager.h"

static void InitWorld(WorldInternal* world) {
	GLUtils::Initialize();
	UniformBufferManager::Initialize();

	Shadows::get();
	MatrixBuffer::get();
	Resources::get()->Import();

	world->root_ = Factory::Create<EntityInternal>();
	world->root_->SetTransform(Factory::Create<TransformInternal>());
	world->root_->SetName("Root");
}

World& WorldInstance() {
	static World instance;
	if (!instance) {
		instance = Factory::Create<WorldInternal>();
		InitWorld(dynamic_cast<WorldInternal*>(instance.get()));
	}

	return instance;
}

bool WorldInternal::LightComparer::operator()(const Light& lhs, const Light& rhs) const {
	// Directional light > Importance > Luminance.
	ObjectType lt = lhs->GetType(), rt = rhs->GetType();
	if (lt != rt && (lt == ObjectTypeDirectionalLight || rt == ObjectTypeDirectionalLight)) {
		return lt == ObjectTypeDirectionalLight;
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
	: ObjectInternal(ObjectTypeWorld), importer_(MEMORY_CREATE(EntityLoaderThreadPool))
	, environment_(MEMORY_CREATE(EnvironmentInternal)) , decals_(SUEDE_MAX_DECALS) {
	Screen::AddScreenSizeChangedListener(this);
	AddEventListener(this);

	screenRenderTarget_.reset(MEMORY_CREATE(ScreenRenderTextureInternal));
	screenRenderTarget_->Create(RenderTextureFormatRgba, Screen::GetWidth(), Screen::GetHeight());

	update_entities = Profiler::get()->CreateSample();
	update_decals = Profiler::get()->CreateSample();
	update_rendering = Profiler::get()->CreateSample();
}

WorldInternal::~WorldInternal() {
}

void WorldInternal::Destroy() {
	for (CameraContainer::iterator ite = cameras_.begin(); ite != cameras_.end(); ++ite) {
		(*ite)->OnBeforeWorldDestroyed();
	}

	UniformBufferManager::Destroy();
	MEMORY_RELEASE(importer_);

	RemoveEventListener(this);
	Screen::RemoveScreenSizeChangedListener(this);

	Profiler::get()->ReleaseSample(update_entities);
	Profiler::get()->ReleaseSample(update_decals);
	Profiler::get()->ReleaseSample(update_rendering);
}

Object WorldInternal::Create(ObjectType type) {
	Object object = Factory::Create(type);
	if (type >= ObjectTypeEntity) {
		Entity entity = suede_dynamic_cast<Entity>(object);
		Transform transform = Factory::Create<TransformInternal>();
		entity->SetTransform(transform);

		EntityCreatedEventPointer e = NewWorldEvent<EntityCreatedEventPointer>();
		e->entity = entity;
		FireEvent(e);

 		GUARD_SCOPE_TYPED(Transform);
 		entities_.insert(std::make_pair(entity->GetInstanceID(), entity));
	}

	if (type >= ObjectTypeSpotLight && type <= ObjectTypeDirectionalLight) {
		lights_.insert(suede_dynamic_cast<Light>(object));
	}

	if (type == ObjectTypeCamera) {
		cameras_.insert(suede_dynamic_cast<Camera>(object));
	}

	if (type == ObjectTypeProjector) {
		projectors_.insert(suede_dynamic_cast<Projector>(object));
	}

	return object;
}

Entity WorldInternal::Import(const std::string& path, EntityLoadedListener* listener) {
	importer_->SetLoadedListener(listener);
	return importer_->Import(path);
}

bool WorldInternal::ImportTo(Entity entity, const std::string& path, EntityLoadedListener* listener) {
	importer_->SetLoadedListener(listener);
	return importer_->ImportTo(entity, path);
}

Entity WorldInternal::GetEntity(uint id) {
	EntityDictionary::iterator ite = entities_.find(id);
	if (ite == entities_.end()) { return nullptr; }
	return ite->second;
}

bool WorldInternal::GetEntities(ObjectType type, std::vector<Entity>& entities) {
	if (type < ObjectTypeEntity) {
		Debug::LogError("invalid entity type");
		return false;
	}

	if (type == ObjectTypeEntity) {
		for (EntityDictionary::iterator ite = entities_.begin(); ite != entities_.end(); ++ite) {
			entities.push_back(ite->second);
		}
	}
	else if (type == ObjectTypeCamera) {
		entities.assign(cameras_.begin(), cameras_.end());
	}
	else if (type == ObjectTypeLights) {
		entities.assign(lights_.begin(), lights_.end());
	}
	else if (type == ObjectTypeProjector) {
		entities.assign(projectors_.begin(), projectors_.end());
	}
	else {
		for (EntityDictionary::iterator ite = entities_.begin(); ite != entities_.end(); ++ite) {
			if (ite->second->GetType() == type) {
				entities.push_back(ite->second);
			}
		}
	}

	return !entities.empty();
}

void WorldInternal::WalkEntityHierarchy(WorldEntityWalker* walker) {
	GUARD_SCOPE_TYPED(Transform);
	WalkEntityHierarchyRecursively(GetRootTransform(), walker);
}

void WorldInternal::AddEventListener(WorldEventListener* listener) {
	if (listener == nullptr) {
		Debug::LogError("invalid world event listener.");
		return;
	}

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

bool WorldInternal::FireEvent(WorldEventBasePointer e) {
	WorldEventType type = e->GetEventType();
	WorldEventCollection& collection = events_[type];

	GUARD_SCOPE_TYPED(WorldEventContainer);
	if (collection.find(e) == collection.end()) {
		collection.insert(e);
		return true;
	}

	return false;
}

void WorldInternal::FireEventImmediate(WorldEventBasePointer e) {
	for (int i = 0; i < listeners_.size(); ++i) {
		listeners_[i]->OnWorldEvent(e);
	}
}

void WorldInternal::GetDecals(std::vector<Decal*>& container) {
	for (DecalContainer::iterator ite = decals_.begin(); ite != decals_.end(); ++ite) {
		container.push_back(*ite);
	}
}

void WorldInternal::RenderUpdate() {
	// TODO: CLEAR STENCIL BUFFER.
	//Framebuffer0::Get()->Clear(FramebufferClearMaskColorDepthStencil);

	UpdateTimeUniformBuffer();

	for (CameraContainer::iterator ite = cameras_.begin(); ite != cameras_.end(); ++ite) {
		if ((*ite)->GetActive()) {
			(*ite)->Render();
		}
	}
}

void WorldInternal::UpdateDecals() {
	decals_.clear();
	CreateDecals(*cameras_.begin());
}

void WorldInternal::UpdateEntities() {
	for (EntityDictionary::iterator ite = entities_.begin(); ite != entities_.end(); ++ite) {
		if (ite->second->GetActive()) {
			ite->second->Update();
		}
	}
}

bool WorldInternal::WalkEntityHierarchyRecursively(Transform root, WorldEntityWalker* walker) {
	int childCount = root->GetChildCount();
	for (int i = 0; i < childCount; ++i) {
		Entity child = root->GetChildAt(i)->GetEntity();
		if (!child) {
			Debug::Break();
			continue;
		}

		WorldEntityWalker::WalkCommand command = walker->OnWalkEntity(child);

		// next sibling.
		if (command == WorldEntityWalker::WalkCommandNext) {
			continue;
		}

		// 
		if (command == WorldEntityWalker::WalkCommandBreak) {
			return false;
		}

		if (!WalkEntityHierarchyRecursively(child->GetTransform(), walker)) {
			return false;
		}
	}

	return true;
}

void WorldInternal::OnScreenSizeChanged(uint width, uint height) {
}

void WorldInternal::OnWorldEvent(WorldEventBasePointer e) {
	if (e->GetEventType() == WorldEventTypeCameraDepthChanged) {
		cameras_.sort();
	}
	else if (e->GetEventType() == WorldEventTypeEntityParentChanged) {
		Entity entity = suede_static_cast<EntityParentChangedEventPointer>(e)->entity;
		if (entity->GetTransform()->GetParent()) {
			entities_.insert(std::make_pair(entity->GetInstanceID(), entity));
		}
		else {
			entities_.erase(entity->GetInstanceID());
		}
	}
}

void WorldInternal::FireEvents() {
	for (uint i = 0; i < WorldEventTypeCount; ++i) {
		WorldEventCollection collection = events_[i];
		for (WorldEventCollection::const_iterator ite = collection.begin(); ite != collection.end(); ++ite) {
			FireEventImmediate(*ite);
		}
	}

	GUARD_SCOPE_TYPED(WorldEventContainer);
	for (uint i = 0; i < WorldEventTypeCount; ++i) {
		events_[i].clear();
	}
}

void WorldInternal::CreateDecals(Camera camera) {
	for (ProjectorContainer::iterator ite = projectors_.begin(); ite != projectors_.end(); ++ite) {
		Projector p = *ite;
		GeometryUtility::CalculateFrustumPlanes(planes_, p->GetProjectionMatrix() * p->GetTransform()->GetWorldToLocalMatrix());

		if (!CreateProjectorDecal(camera, p, planes_)) {
			break;
		}
	}
}

bool WorldInternal::CreateEntityDecal(Camera camera, Decal& decal, Entity entity, Plane planes[6]) {
	std::vector<glm::vec3> triangles;
	if (!ClampMesh(camera, triangles, entity, planes)) {
		return false;
	}

	std::vector<uint> indexes;
	indexes.reserve(triangles.size());
	for (uint i = 0; i < triangles.size(); ++i) {
		indexes.push_back(i);
	}

	decal.indexes = indexes;
	decal.positions = triangles;
	decal.topology = MeshTopology::Triangles;

	return true;
}

bool WorldInternal::CreateProjectorDecal(Camera camera, Projector p, Plane planes[6]) {
	/*std::vector<Entity> entities;
	if (!GetVisibleEntities(entities, p->GetProjectionMatrix() * p->GetTransform()->GetWorldToLocalMatrix())) {
		return true;
	}

	for (std::vector<Entity>::iterator ite = entities.begin(); ite != entities.end(); ++ite) {
		Entity entity = *ite;
		if (entity == p) { continue; }
		
		Decal* decal = decals_.spawn();
		if (decal == nullptr) {
			Debug::LogError("too many decals");
			return false;
		}

		if (!CreateEntityDecal(camera, *decal, entity, planes)) {
			decals_.recycle(decal);
		}

		decal->texture = p->GetTexture();
		decal->matrix = p->GetProjectionMatrix() * p->GetTransform()->GetWorldToLocalMatrix();
	}
	*/
	return true;
}

bool WorldInternal::ClampMesh(Camera camera, std::vector<glm::vec3>& triangles, Entity entity, Plane planes[6]) {
	Mesh mesh = entity->GetMesh();
	glm::vec3 cameraPosition = entity->GetTransform()->InverseTransformPoint(camera->GetTransform()->GetPosition());

	uint* indexes = mesh->MapIndexes();
	glm::vec3* vertices = mesh->MapVertices();

	for (int i = 0; i < mesh->GetSubMeshCount(); ++i) {
		SubMesh subMesh = mesh->GetSubMesh(i);
		const TriangleBias& bias = subMesh->GetTriangleBias();

		// TODO: use triangle strip?
		for (int j = 0; j < bias.indexCount; j += 3) {
			std::vector<glm::vec3> polygon;
			uint index0 = indexes[bias.baseIndex + j] + bias.baseVertex;
			uint index1 = indexes[bias.baseIndex + j + 1] + bias.baseVertex;
			uint index2 = indexes[bias.baseIndex + j + 2] + bias.baseVertex;
			
			glm::vec3 vs[] = { vertices[index0], vertices[index1], vertices[index2] };

			if (!GeometryUtility::IsFrontFace(vs, cameraPosition)) {
				continue;
			}

			vs[0] = entity->GetTransform()->TransformPoint(vs[0]);
			vs[1] = entity->GetTransform()->TransformPoint(vs[1]);
			vs[2] = entity->GetTransform()->TransformPoint(vs[2]);

			GeometryUtility::ClampTriangle(polygon, vs, planes, 6);
			GeometryUtility::Triangulate(triangles, polygon, glm::cross(vs[1] - vs[0], vs[2] - vs[1]));
		}
	}

	mesh->UnmapIndexes();
	mesh->UnmapVertices();

	return triangles.size() >= 3;
}

inline void WorldInternal::UpdateTimeUniformBuffer() {
	static SharedTimeUniformBuffer p;
	p.time.x = Time::get()->GetRealTimeSinceStartup();
	p.time.y = Time::get()->GetDeltaTime();
	UniformBufferManager::UpdateSharedBuffer(SharedTimeUniformBuffer::GetName(), &p, 0, sizeof(p));
}

void WorldInternal::Update() {
	//Debug::StartSample();
	//Debug::StartSample();
	FireEvents();
	//Debug::Output("[events]\t%.3f\n", Debug::EndSample());
	
	update_entities->Restart();
	UpdateEntities();
	update_entities->Stop();
	Debug::Output("[WorldInternal::Update::update_entities]\t%.3f", update_entities->GetElapsedSeconds());
	
	update_decals->Restart();
	UpdateDecals();
	update_decals->Stop();
	Debug::Output("[WorldInternal::Update::update_decals]\t%.3f", update_decals->GetElapsedSeconds());
	
	update_rendering->Restart();
	RenderUpdate();
	update_rendering->Stop();
	Debug::Output("[WorldInternal::Update::update_rendering]\t%.3f", update_rendering->GetElapsedSeconds());

	//Debug::Output("[#total]\t%.3f", Debug::EndSample());
}
