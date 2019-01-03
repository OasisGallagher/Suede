#pragma once
#include <vector>
#include "enginedefines.h"
#include "tools/singleton.h"

class ScreenSizeChangedListener {
public:
	virtual void OnScreenSizeChanged(uint width, uint height) = 0;
};

class SUEDE_API Screen : private singleton2<Screen> {
	friend class singleton<Screen>;
	SUEDE_DECLARE_IMPLEMENTATION(Screen)

public:
	static uint GetWidth();
	static uint GetHeight();

	static void AddScreenSizeChangedListener(ScreenSizeChangedListener* listener);
	static void RemoveScreenSizeChangedListener(ScreenSizeChangedListener* listener);
	static void Resize(uint width, uint height);

private:
	Screen();
};
