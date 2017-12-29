#pragma once
#include "enginedefines.h"

class SUEDE_API Screen {
public:
	static uint GetWidth();
	static uint GetHeight();

private:
	friend class Engine;
	static void Set(uint w, uint h);

private:
	Screen();
};
