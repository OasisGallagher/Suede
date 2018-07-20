#pragma once
#include <vector>
#include "enginedefines.h"
#include "tools/singleton.h"

class ScreenSizeChangedListener {
public:
	virtual void OnScreenSizeChanged(uint width, uint height) = 0;
};

class SUEDE_API Screen : public Singleton<Screen> {
	friend class Singleton<Screen>;

public:
	uint GetWidth() { return width_; }
	uint GetHeight() { return height_; }

	void AddScreenSizeChangedListener(ScreenSizeChangedListener* listener);
	void RemoveScreenSizeChangedListener(ScreenSizeChangedListener* listener);

public:
	void Set(uint width, uint height);

private:
	Screen() {}

private:
	uint width_;
	uint height_;
	std::vector<ScreenSizeChangedListener*> listeners_;
};
