#include "worldinternal.h"

#include "time2.h"
#include "profiler.h"
#include "decalcreater.h"

#include "rigidbody.h"
#include "gameobject.h"

#include "sceneinternal.h"
#include "gizmos.h"
#include "graphics.h"
#include "resources.h"

#include "internal/rendering/shadowmap.h"
#include "internal/codec/gameObjectloader.h"
#include "internal/rendering/renderingcontext.h"
#include "internal/components/transforminternal.h"
#include "internal/rendering/shareduniformbuffers.h"

sorted_event<>& World::frameEnter() { static sorted_event<> e; return e; }
sorted_event<>& World::frameLeave() { static sorted_event<> e; return e; }
event<GameObject*, const std::string&> World::gameObjectImported;

World::World() : Singleton2<World>(new WorldInternal, t_delete<WorldInternal>) {}

void World::Initialize() { _suede_dinstance()->Initialize(); }
void World::Finalize() { _suede_dinstance()->Finalize(); }
void World::CullingUpdate() { _suede_dinstance()->CullingUpdate(); }
void World::Update() { _suede_dinstance()->Update(); }
void World::DestroyGameObject(uint id) { _suede_dinstance()->DestroyGameObject(id); }
void World::DestroyGameObject(GameObject* go) { _suede_dinstance()->DestroyGameObject(go); }
GameObject* World::Import(const std::string& path) { return _suede_dinstance()->Import(path); }
GameObject* World::Import(const std::string& path, Lua::Func<void, GameObject*, const std::string&> callback) { return _suede_dinstance()->Import(path, callback); }
Environment* World::GetEnvironment() { return _suede_dinstance()->GetEnvironment(); }
const FrameStatistics* World::GetFrameStatistics() { return _suede_dinstance()->GetFrameStatistics(); }
void World::ImportTo(GameObject* go, const std::string& path) { _suede_dinstance()->ImportTo(go, path); }
Transform* World::GetRootTransform() { return _suede_dinstance()->GetRootTransform(); }
GameObject* World::GetGameObject(uint id) { return _suede_dinstance()->GetGameObject(id); }
void World::WalkGameObjectHierarchy(GameObjectWalker* walker) { _suede_dinstance()->WalkGameObjectHierarchy(walker); }
void World::GetDecals(std::vector<Decal>& container) { _suede_dinstance()->GetDecals(container); }
std::vector<GameObject*> World::GetGameObjectsOfComponent(suede_guid guid) { return _suede_dinstance()->GetGameObjectsOfComponent(guid); }

WorldInternal::WorldInternal() {
	importer_ = new GameObjectLoaderThreadPool(World::gameObjectImported);
}

void WorldInternal::Initialize() {
	threadId_ = std::this_thread::get_id();

	context_ = new RenderingContext();
	Context::SetCurrent(context_);

	scene_ = new SceneInternal();
	environment_ = new Environment();
}

WorldInternal::~WorldInternal() {
}

void WorldInternal::Finalize() {
	delete scene_;
	delete importer_;
	delete environment_;

	delete context_;

	threadId_ = std::thread::id();

	Screen::sizeChanged.unsubscribe(this);
}

const FrameStatistics* WorldInternal::GetFrameStatistics() {
	return &context_->GetStatistics()->GetFrameStatistics();
}

Transform* WorldInternal::GetRootTransform() {
	return scene_->GetRootTransform();
}

GameObject* WorldInternal::Import(const std::string& path) {
	return importer_->Import(path, nullptr).get();
}

GameObject* WorldInternal::Import(const std::string& path, Lua::Func<void, GameObject*, const std::string&> callback) {
	return importer_->Import(path, callback).get();
}

void WorldInternal::ImportTo(GameObject* go, const std::string& path) {
	return importer_->ImportTo(go, path, nullptr);
}

GameObject* WorldInternal::GetGameObject(uint id) {
	return scene_->GetGameObject(id);
}

void WorldInternal::DestroyGameObject(uint id) {
	scene_->DestroyGameObject(id);
}

void WorldInternal::DestroyGameObject(GameObject* go) {
	scene_->DestroyGameObject(go);
}

std::vector<GameObject*> WorldInternal::GetGameObjectsOfComponent(suede_guid guid) {
	return scene_->GetGameObjectsOfComponent(guid);
}

void WorldInternal::WalkGameObjectHierarchy(GameObjectWalker* walker) {
	scene_->WalkGameObjectHierarchy(walker);
}

void WorldInternal::GetDecals(std::vector<Decal>& container) {
	scene_->GetDecals(container);
}

void WorldInternal::UpdateTimeUniformBuffer() {
	static SharedTimeUniformBuffer p;
	p.time.x = Time::GetRealTimeSinceStartup();
	p.time.y = Time::GetDeltaTime();
}

void WorldInternal::CullingUpdate() {
	context_->CullingUpdate();
	scene_->CullingUpdate();
}

void WorldInternal::Update() {
	uint64 start = Profiler::GetTimeStamp();

	World::frameEnter().raise();

	UpdateTimeUniformBuffer();

	context_->Update();

	scene_->Update();

	World::frameLeave().raise();

	context_->GetStatistics()->SetRenderingElapsed(
		Profiler::TimeStampToSeconds(Profiler::GetTimeStamp() - start)
	);
}
