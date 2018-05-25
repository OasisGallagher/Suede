#include <glm/glm.hpp>
#include <ZThread/Guard.h>

#include "debug/debug.h"
#include "memory/memory.h"
#include "asyncentityimporter.h"

AsyncEntityImporter::AsyncEntityImporter() : listener_(nullptr), executor_(16) {
	Engine::AddFrameEventListener(this);
}

AsyncEntityImporter::~AsyncEntityImporter() {
	Engine::RemoveFrameEventListener(this);

	executor_.interrupt();
	executor_.wait();
}

void AsyncEntityImporter::SetImportedListener(EntityImportedListener* listener) {
	listener_ = listener;
}

void AsyncEntityImporter::OnLoadFinished(EntityAssetLoader* loader) {
	ZThread::Guard<ZThread::Mutex> guard(taskContainerMutex_);

	for (std::vector<ZThread::Task>::iterator ite = tasks_.begin(); ite != tasks_.end(); ++ite) {
		ZThread::Task& task = *ite;
		if (loader == task.operator->()) {
			tasks_.erase(ite);
			schedules_.add(task);
			break;
		}
	}
}

void AsyncEntityImporter::OnFrameEnter() {
	for (; !schedules_.empty();) {
		ZThread::Task schedule = schedules_.next();
		EntityAssetLoader* loader = (EntityAssetLoader*)schedule.operator->();

		if (loader->GetEntity()->GetStatus() != EntityStatusDestroyed) {
			EntityAsset asset = loader->GetEntityAsset();
			for (uint i = 0; i < asset.materialAssets.size(); ++i) {
				asset.materialAssets[i].ApplyAsset();
			}

			loader->GetSurface()->SetAttribute(asset.meshAsset);
			loader->GetEntity()->SetStatus(EntityStatusReady);
		}

		if (listener_ != nullptr) {
			listener_->OnEntityImported(loader->GetEntity(), loader->GetPath());
		}
	}
}

Entity AsyncEntityImporter::Import(const std::string& path) {
	Entity root = NewEntity();
	ImportTo(root, path);
	return root;
}

bool AsyncEntityImporter::ImportTo(Entity entity, const std::string& path) {
	if (!entity) {
		Debug::LogError("invalid entity");
		return false;
	}

	ZThread::Task task = new EntityAssetLoader(path, entity, this);
	tasks_.push_back(task);

	executor_.execute(task);
	return true;
}
