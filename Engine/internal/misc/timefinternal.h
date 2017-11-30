#include <time.h>

#include "time2.h"
extern Time timeInstance;

class TimeInternal : public ITime {
public:
	TimeInternal();

public:
	virtual float GetDeltaTime();
	virtual float GetRealTimeSinceStartup();

	virtual uint GetFrameCount();

	virtual void Update();

private:
	uint frames_;
	clock_t clocks_;
	float deltaTime_;
};
