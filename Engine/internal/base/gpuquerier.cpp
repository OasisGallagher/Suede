#include "gpuquerier.h"

#include "api/gl.h"

GpuQuerier::GpuQuerier() : queriers_(MaxQueries) {
	Engine::AddFrameEventListener(this);

	GL::GenQueries(MaxQueries, ids_);
}

GpuQuerier::~GpuQuerier() {
	Engine::RemoveFrameEventListener(this);
	GL::DeleteQueries(MaxQueries, ids_);
}

uint GpuQuerier::Start(QueryType type, QuerierResultListener* listener) {
	if (listener == nullptr) {
		Debug::LogError("invalid listener");
		return 0;
	}

	return 0;
}

void GpuQuerier::Stop() {
}

void GpuQuerier::OnFrameEnter() {
	for (QuerierContainer::iterator ite = queriers_.begin(); ite != queriers_.end(); ++ite) {
	}
}

void GpuQuerier::OnFrameLeave() {
}
