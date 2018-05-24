#include <glm/glm.hpp>

#include "debug/debug.h"
#include "memory/memory.h"
#include "asyncentityimporter.h"

AsyncEntityImporter::AsyncEntityImporter()
	: loader_(MEMORY_CREATE(EntityAssetLoader))
	, listener_(nullptr)
	, status_(Loader::Running) {
	Engine::AddFrameEventListener(this);

	loader_->SetLoadedListener(this);
	int err = loader_->Start();

	if (err != 0) {
		status_ = Loader::Failed;
		Debug::LogError("failed to start thread: %s", StrError(err).c_str());
	}
}

AsyncEntityImporter::~AsyncEntityImporter() {
	if (loader_->isRunning()) {
		loader_->Terminate();
	}

	for (; loader_->isRunning();) {
		ZThread::Thread::YieldCurrentThread();
	}

	MEMORY_RELEASE(loader_);
	Engine::RemoveFrameEventListener(this);
}

void AsyncEntityImporter::SetImportedListener(EntityImportedListener* listener) {
	listener_ = listener;
}

void AsyncEntityImporter::OnLoadFinished() {
	status_ = loader_->GetStatus();
}

void AsyncEntityImporter::OnFrameEnter() {
	if (status_ == Loader::Running) {
		return;
	}

	EntityAsset asset = loader_->GetEntityAsset();
	for (uint i = 0; i < asset.materialAssets.size(); ++i) {
		asset.materialAssets[i].ApplyAsset();
	}

	loader_->GetSurface()->SetAttribute(asset.meshAsset);

	root_->SetStatus(EntityStatusReady);

	if (listener_ != nullptr) {
		listener_->OnEntityImported(status_ == Loader::Ok, root_);
	}

	root_.reset();
	status_ = Loader::Running;
}

Entity AsyncEntityImporter::Import(const std::string& path) {
	root_ = NewEntity();
	ImportTo(root_, path);
	return root_;
}

bool AsyncEntityImporter::ImportTo(Entity entity, const std::string& path) {
	if (!entity) {
		Debug::LogError("invalid entity");
		return false;
	}

	loader_->Load(path, entity);
	return true;
}

std::string AsyncEntityImporter::StrError(int err) {
	if (err == -1) {
		return "errno = " + std::to_string(errno);
	}

	return "error = " + std::to_string(err);
}
