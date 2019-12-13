#pragma once
#include <stack>

#include "gl.h"
#include "engine.h"
#include "tools/event.h"
#include "tools/singleton.h"
#include "containers/freelist.h"

enum class QueryType {
	SamplesPassed,
	AnySamplesPassed,
	AnySamplesPassedConservative,
	TimeElapsed,
};

class GpuQuerier : public Singleton<GpuQuerier> {
	friend class Singleton<GpuQuerier>;

public:
	uint Start(QueryType type);
	void Stop();
	uint Wait(uint id);
	void Cancel(uint id);

public:
	event<uint, uint> querierReturned;

private:
	GpuQuerier();
	~GpuQuerier();

private:
	void OnFrameEnter();

private:
	struct Querier {
		uint id;
		GLenum type;
	};

	enum {
		MaxQueries = 16,
	};

	typedef free_list<Querier> QuerierContainer;

private:
	Querier* FindQuerier(uint id);

	bool UpdateQuerier(Querier* querier);
	void StopQuerier(Querier* querier);
	void CancelQuerier(Querier* querier);
	void RecycleQuerier(Querier* querier);
	void StartQuerier(Querier* querier, QueryType type);

	GLenum QueryTypeToGLenum(QueryType type);

private:
	uint ids_[MaxQueries];

	QuerierContainer queriers_;
	std::stack<Querier*> stack_;
};
