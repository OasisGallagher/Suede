#include "profilerinternal.h"

#include "time2.h"
#include "engine.h"
#include "frameevents.h"
#include "debug/debug.h"
#include "containers/freelist.h"
#include "internal/base/context.h"

Sample::Sample() : PimplIdiom(new SampleInternal, t_delete<SampleInternal>) {}
void Sample::Start() { _suede_dptr()->Start(); }
void Sample::Restart() { _suede_dptr()->Restart(); }
void Sample::Stop() { _suede_dptr()->Stop(); }
void Sample::Reset() { _suede_dptr()->Reset(); }
double Sample::GetElapsedSeconds() const { return _suede_dptr()->GetElapsedSeconds(); }

#define lastFrameStats	stats_[0]
#define thisFrameStats	stats_[1]
#define FPS_REFRESH_TIME	0.2f

Profiler::Profiler() : Subsystem(new ProfilerInternal(&querierReturned)){}
Sample* Profiler::CreateSample() { return _suede_dptr()->CreateSample(); }
void Profiler::ReleaseSample(Sample* value) { _suede_dptr()->ReleaseSample(value); }
uint Profiler::StartGPUQuery(GPUQueryType type) { return _suede_dptr()->StartGPUQuery(type); }
void Profiler::StopGPUQuery() { _suede_dptr()->StopGPUQuery(); }
uint Profiler::WaitGPUQueryToFinish(uint id) { return _suede_dptr()->WaitGPUQueryToFinish(id); }
void Profiler::CancelGPUQuery(uint id) { _suede_dptr()->CancelGPUQuery(id); }
void Profiler::AddTriangles(uint n) { _suede_dptr()->AddTriangles(n); }
void Profiler::AddDrawcalls(uint n) { _suede_dptr()->AddDrawcalls(n); }
void Profiler::SetScriptElapsed(double value) { _suede_dptr()->SetScriptElapsed(value); }
void Profiler::SetCullingElapsed(double value) { _suede_dptr()->SetCullingElapsed(value); }
void Profiler::SetRenderingElapsed(double value) { _suede_dptr()->SetRenderingElapsed(value); }
const StatisticInfo* Profiler::GetStatisticInfo() { return _suede_dptr()->GetStatisticInfo(); }
void Profiler::Awake() { _suede_dptr()->Awake(); }
void Profiler::Update(float deltaTime) { _suede_dptr()->Update(deltaTime); }

void SampleInternal::Start() {
	started_ = true;
	timeStamp_ = Time::GetTimeStamp();
}

void SampleInternal::Restart() {
	Reset();
	Start();
}

void SampleInternal::Stop() {
	started_ = false;
	elapsed_ += (Time::GetTimeStamp() - timeStamp_);
}

void SampleInternal::Reset() {
	started_ = false;
	elapsed_ = timeStamp_ = 0;
}

double SampleInternal::GetElapsedSeconds() const {
	if (started_) {
		Debug::LogError("call Stop() first.");
		return 0.0;
	}

	return Time::TimeStampToSeconds(elapsed_);
}

ProfilerInternal::ProfilerInternal(event<uint, uint>* querierReturnedEvent) 
	: samples_(1024), gpuQueriers_(MaxGPUQueriers), querierReturnedEvent_(querierReturnedEvent){
	memset(stats_, 0, sizeof(StatisticInfo) * SUEDE_COUNTOF(stats_));
	Engine::GetSubsystem<FrameEvents>()->frameEnter.subscribe(this, &ProfilerInternal::OnFrameEnter, (int)FrameEventQueue::User);
}

ProfilerInternal::~ProfilerInternal() {
	context_->DeleteQueries(MaxGPUQueriers, gpuQuerierIDs_);
	Engine::GetSubsystem<FrameEvents>()->frameEnter.unsubscribe(this);
}

void ProfilerInternal::Awake() {
	context_ = Context::GetCurrent();
	context_->GenQueries(MaxGPUQueriers, gpuQuerierIDs_);

	int index = 0;
	for (auto ite = gpuQueriers_.fbegin(); ite != gpuQueriers_.fend(); ++ite) {
		GPUQuerier* ptr = *ite;
		ptr->id = gpuQuerierIDs_[index++];
	}
}

Sample* ProfilerInternal::CreateSample() {
	std::lock_guard<std::mutex> lock(sampleContainerMutex_);
	return samples_.spawn();
}

void ProfilerInternal::ReleaseSample(Sample* sample) {
	sample->Reset();

	std::lock_guard<std::mutex> lock(sampleContainerMutex_);
	samples_.recycle(sample);
}

void ProfilerInternal::Update(float deltaTime) {
	lastFrameStats = thisFrameStats;
	memset(&thisFrameStats, 0, sizeof(thisFrameStats));

	if (timeCounter_ < FPS_REFRESH_TIME) {
		timeCounter_ += deltaTime;
		frameCounter_++;
	}
	else {
		frameRate_ = (float)frameCounter_ / timeCounter_;
		frameCounter_ = 0;
		timeCounter_ = 0.0f;
	}
}

void ProfilerInternal::SetScriptElapsed(double value) {
	thisFrameStats.scriptElapsed = value;
}

void ProfilerInternal::SetCullingElapsed(double value) {
	thisFrameStats.cullingElapsed = value;
}

void ProfilerInternal::SetRenderingElapsed(double value) {
	thisFrameStats.renderingElapsed = value;
}

const StatisticInfo* ProfilerInternal::GetStatisticInfo() {
	lastFrameStats.frameRate = frameRate_;
	return &lastFrameStats;
}

void ProfilerInternal::AddTriangles(uint n) {
	thisFrameStats.ntriangles += n;
}

void ProfilerInternal::AddDrawcalls(uint n) {
	thisFrameStats.ndrawcalls += n;
}

uint ProfilerInternal::StartGPUQuery(GPUQueryType type) {
	GPUQuerier* querier = gpuQueriers_.spawn();
	if (querier == nullptr) {
		return 0;
	}

	gpuQuerierStack_.push(querier);
	StartGPUQuerier(querier, type);
	return querier->id;
}

void ProfilerInternal::StopGPUQuery() {
	if (gpuQuerierStack_.empty()) {
		Debug::LogError("start & stop mismatch.");
		return;
	}

	StopGPUQuerier(gpuQuerierStack_.top());
	gpuQuerierStack_.pop();
}

uint ProfilerInternal::WaitGPUQueryToFinish(uint id) {
	GPUQuerier* querier = FindGPUQuerier(id);
	if (querier != nullptr) {
		uint result = 0;
		context_->GetQueryObjectuiv(querier->id, GL_QUERY_RESULT, &result);
		RecycleGPUQuerier(querier);
		return result;
	}

	return 0;
}

void ProfilerInternal::CancelGPUQuery(uint id) {
	GPUQuerier* querier = FindGPUQuerier(id);
	if (querier != nullptr) {
		CancelGPUQuerier(querier);
	}
}

void ProfilerInternal::OnFrameEnter() {
	for (auto ite = gpuQueriers_.begin(); ite != gpuQueriers_.end(); ) {
		GPUQuerier* querier = *ite;
		if (UpdateGPUQuerier(querier)) {
			RecycleGPUQuerier(*ite++);
		}
		else {
			++ite;
		}
	}
}

bool ProfilerInternal::UpdateGPUQuerier(GPUQuerier* querier) {
	int available = 0;
	context_->GetQueryObjectiv(querier->id, GL_QUERY_RESULT_AVAILABLE, &available);

	if (available) {
		uint result = 0;
		context_->GetQueryObjectuiv(querier->id, GL_QUERY_RESULT, &result);
		querierReturnedEvent_->raise(querier->id, result);

		return true;
	}

	return false;
}

void ProfilerInternal::StartGPUQuerier(GPUQuerier* querier, GPUQueryType type) {
	uint glType = GPUQueryTypeToGLenum(type);

	querier->type = glType;
	context_->BeginQuery(glType, querier->id);
}

void ProfilerInternal::StopGPUQuerier(GPUQuerier* querier) {
	context_->EndQuery(querier->type);
}

void ProfilerInternal::CancelGPUQuerier(GPUQuerier* querier) {
	StopGPUQuerier(querier);
	RecycleGPUQuerier(querier);
}

ProfilerInternal::GPUQuerier* ProfilerInternal::FindGPUQuerier(uint id) {
	for (auto ite = gpuQueriers_.begin(); ite != gpuQueriers_.end(); ++ite) {
		GPUQuerier* querier = *ite;
		if (querier->id == id) {
			return querier;
		}
	}

	Debug::LogError("invalid querier id %u.", id);
	return nullptr;
}

void ProfilerInternal::RecycleGPUQuerier(GPUQuerier* querier) {
	querier->type = 0;
	gpuQueriers_.recycle(querier);
}

uint ProfilerInternal::GPUQueryTypeToGLenum(GPUQueryType type) {
	switch (type) {
	case GPUQueryType::SamplesPassed: return GL_SAMPLES_PASSED;
	case GPUQueryType::AnySamplesPassed: return GL_ANY_SAMPLES_PASSED;
	case GPUQueryType::AnySamplesPassedConservative: return GL_ANY_SAMPLES_PASSED_CONSERVATIVE;
	case GPUQueryType::TimeElapsed: return GL_TIME_ELAPSED;
	}

	Debug::LogError("invalid query type.");
	return 0;
}

