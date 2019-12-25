#include "gpuquerier.h"

#include "world.h"
#include "context.h"
#include "frameeventqueue.h"

GpuQuerier::GpuQuerier(Context* context) : context_(context), queriers_(MaxQueries) {
	World::frameEnter().subscribe(this, &GpuQuerier::OnFrameEnter, (int)FrameEventQueue::User);

	context_->GenQueries(MaxQueries, ids_);

	int index = 0;
	for (QuerierContainer::iterator ite = queriers_.fbegin(); ite != queriers_.fend(); ++ite) {
		Querier* ptr = *ite;
		ptr->id = ids_[index++];
	}
}

GpuQuerier::~GpuQuerier() {
	context_->DeleteQueries(MaxQueries, ids_);
	World::frameEnter().unsubscribe(this);
}

uint GpuQuerier::Start(QueryType type) {
	Querier* querier = queriers_.spawn();
	if (querier == nullptr) {
		return 0;
	}

	stack_.push(querier);
	StartQuerier(querier, type);
	return querier->id;
}

void GpuQuerier::Stop() {
	if (stack_.empty()) {
		Debug::LogError("start & stop mismatch.");
		return;
	}

	StopQuerier(stack_.top());
	stack_.pop();
}

uint GpuQuerier::Wait(uint id) {
	Querier* querier = FindQuerier(id);
	if (querier != nullptr) {
		uint result = 0;
		context_->GetQueryObjectuiv(querier->id, GL_QUERY_RESULT, &result);
		RecycleQuerier(querier);
		return result;
	}

	return 0;
}

void GpuQuerier::Cancel(uint id) {
	Querier* querier = FindQuerier(id);
	if (querier != nullptr) {
		CancelQuerier(querier);
	}
}

void GpuQuerier::OnFrameEnter() {
	for (QuerierContainer::iterator ite = queriers_.begin(); ite != queriers_.end(); ) {
		Querier* querier = *ite;
		if (UpdateQuerier(querier)) {
			RecycleQuerier(*ite++);
		}
		else {
			++ite;
		}
	}
}

bool GpuQuerier::UpdateQuerier(Querier* querier) {
	int available = 0;
	context_->GetQueryObjectiv(querier->id, GL_QUERY_RESULT_AVAILABLE, &available);

	if (available) {
		uint result = 0;
		context_->GetQueryObjectuiv(querier->id, GL_QUERY_RESULT, &result);
		querierReturned.raise(querier->id, result);

		return true;
	}

	return false;
}

void GpuQuerier::StartQuerier(Querier* querier, QueryType type) {
	uint glType = QueryTypeToGLenum(type);

	querier->type = glType;
	context_->BeginQuery(glType, querier->id);
}

void GpuQuerier::StopQuerier(Querier* querier) {
	context_->EndQuery(querier->type);
}

void GpuQuerier::CancelQuerier(Querier* querier) {
	StopQuerier(querier);
	RecycleQuerier(querier);
}

GpuQuerier::Querier* GpuQuerier::FindQuerier(uint id) {
	for (QuerierContainer::iterator ite = queriers_.begin(); ite != queriers_.end(); ++ite) {
		Querier* querier = *ite;
		if (querier->id == id) {
			return querier;
		}
	}

	Debug::LogError("invalid querier id %u.", id);
	return nullptr;
}

void GpuQuerier::RecycleQuerier(Querier* querier) {
	querier->type = 0;
	queriers_.recycle(querier);
}

uint GpuQuerier::QueryTypeToGLenum(QueryType type) {
	switch (type) {
		case QueryType::SamplesPassed: return GL_SAMPLES_PASSED;
		case QueryType::AnySamplesPassed: return GL_ANY_SAMPLES_PASSED;
		case QueryType::AnySamplesPassedConservative: return GL_ANY_SAMPLES_PASSED_CONSERVATIVE;
		case QueryType::TimeElapsed: return GL_TIME_ELAPSED;
	}

	Debug::LogError("invalid query type.");
	return 0;
}
