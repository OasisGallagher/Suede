#pragma once
#include <string>
#include "types.h"
#include "enginedefines.h"

class SUEDE_API Driver {
public:
	static const char* GetVendor();
	static const char* GetRenderer();
	static const char* GetVersion();
	static const char* GetGLSLVersion();

	static bool IsSupported(const char* feature);

private:
	friend class Engine;
	static bool Initialize();

private:
	Driver();
};
