#pragma once
#include "engine.h"
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

class GpuQuerier : public FrameEventListener {
	struct Querier {
		uint id;
		QuerierResultListener* listener;
	};

public:
	GpuQuerier();
	~GpuQuerier();

public:
	static uint Start(QueryType type, QuerierResultListener* listener);
	static void Stop();

protected:
	virtual void OnFrameEnter();
	virtual void OnFrameLeave();

private:
	typedef free_list<Querier> QuerierContainer;
	QuerierContainer queriers_;
};
