#pragma once
#include <vector>
#include "enginedefines.h"
#include "tools/singleton.h"

class ScreenSizeChangedListener {
public:
	virtual void OnScreenSizeChanged(uint width, uint height) = 0;
};

class SUEDE_API Screen : public Singleton2<Screen> {
public:
	virtual uint GetWidth() = 0;
	virtual uint GetHeight() = 0;

	virtual void AddScreenSizeChangedListener(ScreenSizeChangedListener* listener) = 0;
	virtual void RemoveScreenSizeChangedListener(ScreenSizeChangedListener* listener) = 0;
	virtual void Resize(uint width, uint height) = 0;
};
