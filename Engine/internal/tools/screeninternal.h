#pragma once
#include "screen.h"

class ScreenInternal {
public:
	ScreenInternal() :width_(0), height_(0) {}

public:
	uint GetWidth() { return width_; }
	uint GetHeight() { return height_; }

	void Resize(uint width, uint height);

private:
	uint width_;
	uint height_;
};
