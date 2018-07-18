#pragma once
#include "engine.h"
#include "tools/singleton.h"
#include "containers/freelist.h"

enum QueryType {
	QueryTypeSamplesPassed,
	QueryTypeAnySamplesPassed,
	QueryTypeAnySamplesPassedConservative,
	QueryTypeTimeElapsed,
	QueryTypeTimestamp,
};

class QuerierResultListener {
public:
	virtual void OnQuerierResult(uint id) = 0;
};

class GpuQuerier : public FrameEventListener, public Singleton<GpuQuerier> {
public:
	static uint Start(QueryType type, QuerierResultListener* listener);
	static void Stop();

private:
	GpuQuerier();
	~GpuQuerier();

private:
	virtual void OnFrameEnter();
	virtual void OnFrameLeave();

private:
	struct Querier {
		uint id;
		QuerierResultListener* listener;
	};

	enum {
		MaxQueries = 16,
	};

	typedef free_list<Querier> QuerierContainer;

private:
	uint ids_[MaxQueries];
	QuerierContainer queriers_;
};
