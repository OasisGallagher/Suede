#pragma once
#include <set>
#include <map>
#include <mutex>
#include <thread>

#include "engine.h"
#include "light.h"
#include "camera.h"
#include "screen.h"
#include "projector.h"
#include "gameobject.h"
#include "gizmospainter.h"
#include "containers/sortedvector.h"

#include "tools/event.h"

class SceneInternal;
class Sample;
class RenderingContext;
class DecalCreater;

class EngineInternal {
public:
	EngineInternal();
	~EngineInternal() {}

public:
	void Startup(GLCanvas* canvas);
	void Shutdown();

	void BeginFrame();
	void EndFrame();

	void Update();

	Subsystem* GetSubsystem(SubsystemType type);

private:
	void UpdateTimeUniformBuffer();

private:
	GLCanvas* canvas_;

	std::thread::id threadId_;
	RenderingContext* context_;

	std::mutex hierarchyMutex_;

	Subsystem* subsystems_[(int)SubsystemType::_Count];

	sorted_event<>* frameEnterEvent_, *frameLeaveEvent_;
};
