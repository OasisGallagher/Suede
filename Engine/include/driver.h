#pragma once
#include <string>
#include "types.h"
#include "enginedefines.h"

class SUEDE_API Driver {
public:
	static const char* GetVendor();
	static const char* GetRenderer();
	static const char* GetGLSLVersion();
	static const char* GetOpenGLVersion();

	static bool IsSupported(const char* feature);

public: // TODO:
	static bool Initialize();

private:
	Driver();
};
