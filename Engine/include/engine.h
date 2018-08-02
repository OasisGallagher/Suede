#pragma once
#include "enginedefines.h"
#include "tools/singleton.h"
#include "frameeventlistener.h"

class SUEDE_API Engine : public Singleton2<Engine> {
public:
	virtual bool Initialize(uint width, uint height) = 0;
	virtual void Release() = 0;

	virtual void Update() = 0;

	virtual void AddFrameEventListener(FrameEventListener* listener) = 0;
	virtual void RemoveFrameEventListener(FrameEventListener* listener) = 0;
};
