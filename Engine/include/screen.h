#pragma once
#include <vector>
#include "enginedefines.h"
#include "tools/singleton.h"

class ScreenSizeChangedListener {
public:
	virtual void OnScreenSizeChanged(uint width, uint height) = 0;
};

class SUEDE_API Screen : public Singleton2<Screen> {
	friend class Singleton<Screen>;
	SUEDE_DECLARE_IMPLEMENTATION(Screen)

public:
	uint GetWidth();
	uint GetHeight();

	void AddScreenSizeChangedListener(ScreenSizeChangedListener* listener);
	void RemoveScreenSizeChangedListener(ScreenSizeChangedListener* listener);
	void Resize(uint width, uint height);

private:
	Screen();
};
