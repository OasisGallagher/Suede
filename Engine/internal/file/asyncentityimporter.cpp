#include <glm/glm.hpp>
#include <ZThread/Guard.h>

#include "debug/debug.h"
#include "memory/memory.h"
#include "asyncentityimporter.h"

AsyncEntityImporter::AsyncEntityImporter() : listener_(nullptr)
#ifdef USE_POOL_EXECUTOR
	, executor_(16)
#endif
{
	Engine::AddFrameEventListener(this);
}

AsyncEntityImporter::~AsyncEntityImporter() {
	Engine::RemoveFrameEventListener(this);

	try {
		executor_.interrupt();
		executor_.wait();
	}
	catch (const ZThread::Synchronization_Exception& e) {
		Debug::LogError(e.what());
	}
	catch (...) {
		Debug::LogError("unknown exception");
	}
}

void AsyncEntityImporter::SetImportedListener(EntityImportedListener* listener) {
	listener_ = listener;
}

void AsyncEntityImporter::OnLoadFinished(EntityAssetLoader* loader) {
	ZThread::Guard<ZThread::Mutex> guard(scheduleContainerMutex_);

	for (std::vector<ZThread::Task>::iterator ite = tasks_.begin(); ite != tasks_.end(); ++ite) {
		ZThread::Task task = *ite;

		if (loader == task.operator->()) {
			tasks_.erase(ite);
			schedules_.push(task);
			break;
		}
	}
}

void AsyncEntityImporter::OnFrameEnter() {
	try {
		UpdateSchedules();
	}
	catch (const ZThread::Synchronization_Exception& e) {
		Debug::LogError(e.what());
	}
	catch (...) {
		Debug::LogError("unknown exception");
	}
}

void AsyncEntityImporter::UpdateSchedules() {
	ZThread::Guard<ZThread::Mutex> guard(scheduleContainerMutex_);

	for (; !schedules_.empty();) {
		ZThread::Task schedule = schedules_.front();
		schedules_.pop();

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

	try {
		executor_.execute(task);
	}
	catch (const ZThread::Synchronization_Exception& e) {
		Debug::LogError(e.what());
	}

	return true;
}
