#pragma once
#include <string>
#include "types.h"
#include "enginedefines.h"

class SUEDE_API Driver {
public:
	enum Limits {
		RedBits,
		GreenBits,
		BlueBits,
		AlphaBits,
		DepthBits,
		StencilBits,

		MaxClipPlanes,
		MaxTextureSize,
	};

public:
	static const char* GetVendor();
	static const char* GetRenderer();
	static const char* GetVersion();
	static const char* GetGLSLVersion();

	static uint GetLimits(Limits limits);

private:
	Driver();
};
