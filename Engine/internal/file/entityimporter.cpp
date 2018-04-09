#include <glm/glm.hpp>

#include "debug/debug.h"
#include "memory/memory.h"
#include "entityimporter.h"

EntityImporter::EntityImporter() 
	: loader_(MEMORY_CREATE(EntityAssetLoader))
	, status_(Loader::Failed) {
	loader_->SetCallback(this);
}

EntityImporter::~EntityImporter() {
	MEMORY_RELEASE(loader_);
}

void EntityImporter::operator()() {
	status_ = Loader::Ok;
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

	if (status_ != Loader::Failed) {
		Debug::LogError("asset importer is running");
		return false;
	}

	loader_->SetTarget(path, entity);

	int err = loader_->Start();
	if (err != 0) {
		Debug::LogError("failed to start thread: %s", StrError(err).c_str());
		return false;
	}

	return true;
}

void EntityImporter::Update() {
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
}

std::string EntityImporter::StrError(int err) {
	if (err == -1) {
		return "errno = " + std::to_string(errno);
	}

	return "error = " + std::to_string(err);
}

