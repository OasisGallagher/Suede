#pragma once

class FrameEventListener {
public:
	virtual void OnFrameEnter() = 0;
	virtual void OnFrameLeave() = 0;
};
