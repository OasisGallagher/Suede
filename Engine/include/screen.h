#pragma once
#include "enginedefines.h"

class ScreenSizeChangedListener {
public:
	virtual void OnScreenSizeChanged(uint width, uint height) = 0;
};

class SUEDE_API Screen {
public:
	static uint GetWidth();
	static uint GetHeight();

	static void AddScreenSizeChangedListener(ScreenSizeChangedListener* listener);
	static void RemoveScreenSizeChangedListener(ScreenSizeChangedListener* listener);

	static void Set(uint width, uint height);

private:
//	friend class GraphicsViewer;

private:
	Screen();
};
