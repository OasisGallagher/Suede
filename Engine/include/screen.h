#pragma once
#include "types.h"
#include "tools/event.h"

class SUEDE_API Screen {
public:
	static uint GetWidth();
	static uint GetHeight();
	static void Resize(uint width, uint height);

	static event<uint, uint> sizeChanged;
};
