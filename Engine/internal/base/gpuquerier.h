#pragma once
#include <stack>

#include "engine.h"
#include "../api/gl.h"
#include "tools/singleton.h"
#include "containers/freelist.h"

enum QueryType {
	QueryTypeSamplesPassed,
	QueryTypeAnySamplesPassed,
	QueryTypeAnySamplesPassedConservative,
	QueryTypeTimeElapsed,
};

class QuerierResultListener {
public:
	virtual void OnQuerierResult(uint id, uint result) = 0;
};

class GpuQuerier : public FrameEventListener, public Singleton<GpuQuerier> {
	friend class Singleton<GpuQuerier>;

public:
	uint Start(QueryType type, QuerierResultListener* listener);
	void Stop();
	uint Wait(uint id);
	void Cancel(uint id);

private:
	GpuQuerier();
	~GpuQuerier();

private:
	virtual void OnFrameEnter();

private:
	struct Querier {
		uint id;
		GLenum type;
		QuerierResultListener* listener;
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
	void StartQuerier(Querier* querier, QueryType type, QuerierResultListener* listener);

	GLenum QueryTypeToGLenum(QueryType type);

private:
	uint ids_[MaxQueries];

	QuerierContainer queriers_;
	std::stack<Querier*> stack_;
};
