#pragma once
#include <memory>
#include "defines.h"

class ENGINE_EXPORT IScreen {
public:
	virtual int GetContextWidth() = 0;
	virtual int GetContextHeight() = 0;

	virtual void SetContentSize(int w, int h) = 0;
};

typedef std::shared_ptr<IScreen> Screen;
