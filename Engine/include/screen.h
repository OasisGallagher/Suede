#pragma once
#include <vector>
#include "enginedefines.h"

#include "tools/event.h"
#include "tools/singleton.h"

class SUEDE_API Screen : private Singleton2<Screen> {
	friend class Singleton<Screen>;
	SUEDE_DECLARE_IMPLEMENTATION(Screen)

public:
	static uint GetWidth();
	static uint GetHeight();

	static void Resize(uint width, uint height);

public:
	static event<uint, uint> sizeChanged;

private:
	Screen();
};
