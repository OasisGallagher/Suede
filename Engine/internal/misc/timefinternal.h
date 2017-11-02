#include "timef.h"

extern Time timeInstance;

class TimeInternal : public ITime {
public:
	TimeInternal();

public:
	virtual float GetDeltaTime();
	virtual float GetRealTimeSinceStartup();
	virtual int GetFrameCount();
	virtual void Update();

private:
	int frames_;
	clock_t clocks_;
	float deltaTime_;
};
