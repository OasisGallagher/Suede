#include <glm/glm.hpp>

#include "debug/debug.h"
#include "memory/memory.h"
#include "entityimporter.h"

EntityImporter::EntityImporter() 
	: loader_(MEMORY_CREATE(EntityAssetLoader))
	, status_(Loader::Failed) {
	Engine::AddFrameEventListener(this);

	loader_->SetCallback(this);
	int err = loader_->Start();
	if (err != 0) {
		Debug::LogError("failed to start thread: %s", StrError(err).c_str());
	}
}

EntityImporter::~EntityImporter() {
	if (loader_->isRunning()) {
		loader_->Terminate();
	}

	for (; loader_->isRunning();) {
		OpenThreads::Thread::YieldCurrentThread();
	}

	MEMORY_RELEASE(loader_);

	Engine::RemoveFrameEventListener(this);
}

void EntityImporter::OnLoadFinished() {
	status_ = Loader::Ok;
}

void EntityImporter::OnFrameEnter() {
	if (status_ != Loader::Ok) {
		return;
	}

	status_ = Loader::Failed;
	EntityAsset asset = loader_->GetEntityAsset();
	for (uint i = 0; i < asset.materialAssets.size(); ++i) {
		asset.materialAssets[i].ApplyAsset();
	}

	loader_->GetSurface()->SetAttribute(asset.meshAsset);

	typedef std::vector<Renderer> Renderers;
	Renderers renderers = loader_->GetRenderers();
	for (Renderers::iterator ite = renderers.begin(); ite != renderers.end(); ++ite) {
		(*ite)->SetReady(true);
	}

	loader_->root_->GetTransform()->SetPosition(glm::vec3(0, 25, -65));
	loader_->root_->GetTransform()->SetEulerAngles(glm::vec3(30, 60, 0));
	loader_->root_->GetTransform()->SetScale(glm::vec3(0.01f));
}

Entity EntityImporter::Import(const std::string& path) {
	Entity entity = NewEntity();
	ImportTo(entity, path);
	return entity;
}

bool EntityImporter::ImportTo(Entity entity, const std::string& path) {
	if (!entity) {
		Debug::LogError("invalid entity");
		return false;
	}

	loader_->Load(path, entity);
	return true;
}

std::string EntityImporter::StrError(int err) {
	if (err == -1) {
		return "errno = " + std::to_string(errno);
	}

	return "error = " + std::to_string(err);
}

