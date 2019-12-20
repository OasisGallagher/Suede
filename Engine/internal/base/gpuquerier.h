#pragma once
#include <stack>

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

class Context;
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
	GpuQuerier(Context* context);
	~GpuQuerier();

private:
	void OnFrameEnter();

private:
	struct Querier {
		uint id;
		uint type;
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

	uint QueryTypeToGLenum(QueryType type);

private:
	Context* context_;

	uint ids_[MaxQueries];

	QuerierContainer queriers_;
	std::stack<Querier*> stack_;
};
