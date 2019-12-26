#pragma once
#include "defines.h"
#include "subsystem.h"
#include "tools/event.h"

enum class GPUQueryType {
	SamplesPassed,
	AnySamplesPassed,
	AnySamplesPassedConservative,
	TimeElapsed,
};

struct StatisticInfo {
	uint ndrawcalls;
	uint nvertices;
	uint ntriangles;

	float frameRate;

	double scriptElapsed;
	double cullingElapsed;
	double renderingElapsed;
};

class SUEDE_API Sample : public PimplIdiom {
	SUEDE_DECLARE_IMPLEMENTATION(Sample)

public:
	Sample();

public:
	void Start();
	void Restart();

	void Stop();

	void Reset();
	double GetElapsedSeconds() const;
};

class SUEDE_API Profiler : public Subsystem {
	SUEDE_DECLARE_IMPLEMENTATION(Profiler)
public:
	enum {
		SystemType = SubsystemType::Profiler,
	};

public:
	Profiler();

public:
	Sample* CreateSample();
	void ReleaseSample(Sample* value);

	uint StartGPUQuery(GPUQueryType type);
	void StopGPUQuery();
	uint WaitGPUQueryToFinish(uint id);
	void CancelGPUQuery(uint id);

	void AddTriangles(uint n);
	void AddDrawcalls(uint n);

	void SetScriptElapsed(double value);
	void SetCullingElapsed(double value);
	void SetRenderingElapsed(double value);

	const StatisticInfo* GetStatisticInfo();

public:
	event<uint, uint> querierReturned;

public:
	virtual void Awake();
	virtual void Update(float deltaTime);
};
