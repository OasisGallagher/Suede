#pragma once

#include "enginedefines.h"

class SUEDE_API Engine {
public:
	static bool Initialize();
	static void Release();

	static void Update();
	static void Resize(int w, int h);

private:
	static void SetDefaultRenderStates();
};
