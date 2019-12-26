#pragma once
#include <stack>

#include "profiler.h"
#include "subsysteminternal.h"
#include "containers/freelist.h"

class SampleInternal {
public:
	SampleInternal() { Reset(); }

public:
	void Start();
	void Restart();

	void Stop();
	void Reset();

	double GetElapsedSeconds() const;

private:
	bool started_;
	uint64 elapsed_;
	uint64 timeStamp_;
};

class Context;
class ProfilerInternal : public SubsystemInternal {
public:
	ProfilerInternal(event<uint, uint>* querierReturnedEvent);
	~ProfilerInternal();

public:
	Sample* CreateSample();
	void ReleaseSample(Sample* value);

	void AddTriangles(uint n);
	void AddDrawcalls(uint n);

	void SetScriptElapsed(double value);
	void SetCullingElapsed(double value);
	void SetRenderingElapsed(double value);

	const StatisticInfo* GetStatisticInfo();

	uint StartGPUQuery(GPUQueryType type);
	void StopGPUQuery();
	uint WaitGPUQueryToFinish(uint id);
	void CancelGPUQuery(uint id);

public:
	void Awake();
	void Update(float deltaTime);

private:
	void OnFrameEnter();

private:
	struct GPUQuerier {
		uint id;
		uint type;
	};

	enum {
		MaxGPUQueriers = 16,
	};

private:
	GPUQuerier* FindGPUQuerier(uint id);
	bool UpdateGPUQuerier(GPUQuerier* querier);
	void StopGPUQuerier(GPUQuerier* querier);
	void CancelGPUQuerier(GPUQuerier* querier);
	void RecycleGPUQuerier(GPUQuerier* querier);
	void StartGPUQuerier(GPUQuerier* querier, GPUQueryType type);
	uint GPUQueryTypeToGLenum(GPUQueryType type);

private:
	Context* context_;
	event<uint, uint>* querierReturnedEvent_;

	uint gpuQuerierIDs_[MaxGPUQueriers];

	free_list<GPUQuerier> gpuQueriers_;
	std::stack<GPUQuerier*> gpuQuerierStack_;

	free_list<Sample> samples_;
	std::mutex sampleContainerMutex_;

	float frameRate_ = 0;
	float timeCounter_ = 0;
	uint frameCounter_ = 0;

	StatisticInfo stats_[2];
};
