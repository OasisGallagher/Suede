#pragma once
#include "screen.h"

class ScreenInternal : public Screen {
public:
	ScreenInternal(uint width, uint height) :width_(width), height_(height) {
	}

public:
	virtual uint GetWidth() { return width_; }
	virtual uint GetHeight() { return height_; }

	virtual void AddScreenSizeChangedListener(ScreenSizeChangedListener* listener);
	virtual void RemoveScreenSizeChangedListener(ScreenSizeChangedListener* listener);
	virtual void Resize(uint width, uint height);

private:
	uint width_;
	uint height_;
	std::vector<ScreenSizeChangedListener*> listeners_;
};
