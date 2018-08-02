#include "gpuquerier.h"

GpuQuerier::GpuQuerier() : queriers_(MaxQueries) {
	Engine::instance()->AddFrameEventListener(this);

	GL::GenQueries(MaxQueries, ids_);

	int index = 0;
	for (QuerierContainer::iterator ite = queriers_.fbegin(); ite != queriers_.fend(); ++ite) {
		Querier* ptr = *ite;
		ptr->id = ids_[index++];
		ptr->listener = nullptr;
	}
}

GpuQuerier::~GpuQuerier() {
	GL::DeleteQueries(MaxQueries, ids_);
}

uint GpuQuerier::Start(QueryType type, QuerierResultListener* listener) {
	Querier* querier = queriers_.spawn();
	if (querier == nullptr) {
		return 0;
	}

	stack_.push(querier);
	StartQuerier(querier, type, listener);
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
		GL::GetQueryObjectuiv(querier->id, GL_QUERY_RESULT, &result);
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
	GL::GetQueryObjectiv(querier->id, GL_QUERY_RESULT_AVAILABLE, &available);

	if (available) {
		uint result = 0;
		GL::GetQueryObjectuiv(querier->id, GL_QUERY_RESULT, &result);
		if (querier->listener != nullptr) {
			querier->listener->OnQuerierResult(querier->id, result);
		}

		return true;
	}

	return false;
}

void GpuQuerier::StartQuerier(Querier* querier, QueryType type, QuerierResultListener* listener) {
	querier->listener = listener;
	GLenum glType = QueryTypeToGLenum(type);

	querier->type = glType;
	GL::BeginQuery(glType, querier->id);
}

void GpuQuerier::StopQuerier(Querier* querier) {
	GL::EndQuery(querier->type);
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
	querier->listener = nullptr;
	queriers_.recycle(querier);
}

GLenum GpuQuerier::QueryTypeToGLenum(QueryType type) {
	switch (type) {
		case QueryTypeSamplesPassed: return GL_SAMPLES_PASSED;
		case QueryTypeAnySamplesPassed: return GL_ANY_SAMPLES_PASSED;
		case QueryTypeAnySamplesPassedConservative: return GL_ANY_SAMPLES_PASSED_CONSERVATIVE;
		case QueryTypeTimeElapsed: return GL_TIME_ELAPSED;
	}

	Debug::LogError("invalid query type.");
	return 0;
}
