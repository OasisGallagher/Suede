#include "gpuquerier.h"

GpuQuerier::GpuQuerier() : queriers_(16) {
	Engine::AddFrameEventListener(this);
}

GpuQuerier::~GpuQuerier() {
	Engine::RemoveFrameEventListener(this);
}

uint GpuQuerier::Start(QueryType type, QuerierResultListener* listener) {
	return 0;
}

void GpuQuerier::Stop() {

}

void GpuQuerier::OnFrameEnter() {
	for (QuerierContainer::iterator ite = queriers_.begin(); ite != queriers_.end(); ++ite) {
	}
}

void GpuQuerier::OnFrameLeave() {
	throw std::logic_error("The method or operation is not implemented.");
}

