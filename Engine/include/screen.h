#pragma once
#include "defines.h"

class SUEDE_API IScreen {
public:
	virtual int GetContextWidth() = 0;
	virtual int GetContextHeight() = 0;

	virtual void SetContentSize(int w, int h) = 0;
};

SUEDE_DEFINE_OBJECT_POINTER(Screen);
