#pragma once
#include "types.h"

class SUEDE_API GraphicsCanvas {
public:
	virtual void MakeCurrent() = 0;
	virtual void DoneCurrent() = 0;
	virtual void SwapBuffers() = 0;

	virtual uint GetWidth() = 0;
	virtual uint GetHeight() = 0;
};
