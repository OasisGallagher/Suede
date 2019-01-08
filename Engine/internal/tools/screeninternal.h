#pragma once
#include "screen.h"

class ScreenInternal {
public:	ScreenInternal() :width_(0), height_(0) {
	}

public:
	uint GetWidth() { return width_; }
	uint GetHeight() { return height_; }

	void AddScreenSizeChangedListener(ScreenSizeChangedListener* listener);
	void RemoveScreenSizeChangedListener(ScreenSizeChangedListener* listener);
	void Resize(uint width, uint height);

private:
	uint width_;
	uint height_;
	std::vector<ScreenSizeChangedListener*> listeners_;
};
