#ifdef _DEBUG
#include "vld/vld.h"
#endif

#include "engineinternal.h"

#include "time2.h"
#include "profiler.h"
#include "decalcreater.h"
#include "glcanvas.h"

#include "rigidbody.h"
#include "gameobject.h"

#include "tags.h"
#include "input.h"
#include "gizmos.h"
#include "physics.h"
#include "graphics.h"
#include "layermask.h"
#include "resources.h"

#include "frameevents.h"
#include "sceneinternal.h"

#include "internal/rendering/shadowmap.h"
#include "internal/rendering/renderingcontext.h"
#include "internal/components/transforminternal.h"
#include "internal/rendering/shareduniformbuffers.h"

Engine::Engine() : Singleton2<Engine>(new EngineInternal, t_delete<EngineInternal>) {}

void Engine::Startup(GLCanvas* canvas) { _suede_dinstance()->Startup(canvas); }
void Engine::BeginFrame() { _suede_dinstance()->BeginFrame(); }
void Engine::EndFrame() { _suede_dinstance()->EndFrame(); }
void Engine::Shutdown() { _suede_dinstance()->Shutdown(); }
void Engine::Update() { _suede_dinstance()->Update(); }
Subsystem* Engine::GetSubsystem(SubsystemType type) { return _suede_dinstance()->GetSubsystem(type); }

class CullingUpdateTask : public Task {
public:
	CullingUpdateTask() {
		time_ = Engine::GetSubsystem<Time>();
		scene_ = Engine::GetSubsystem<Scene>();
		profiler_ = Engine::GetSubsystem<Profiler>();
	}

	void Run() {
		uint64 start = Time::GetTimeStamp();
		_suede_drptr(scene_)->CullingUpdate(time_->GetDeltaTime());
		profiler_->SetCullingUpdateElapsed(
			Time::TimeStampToSeconds(Time::GetTimeStamp() - start)
		);
	}

	Time* time_;
	Scene* scene_;
	Profiler* profiler_;
};

EngineInternal::EngineInternal() {
	static FrameEvents eventSystem;
	frameEnterEvent_ = &eventSystem.frameEnter;
	frameLeaveEvent_ = &eventSystem.frameLeave;

	subsystems_[(int)SubsystemType::FrameEvents] = &eventSystem;
}

#define REGISTER_SUBSYSTEM(T, ...)	(T*)(subsystems_[(int)SubsystemType::T] = new T(__VA_ARGS__))

void EngineInternal::Startup(GLCanvas* canvas) {
	canvas_ = canvas;
	Screen::Resize(canvas->GetWidth(), canvas->GetHeight());
	threadId_ = std::this_thread::get_id();

	Time* time = REGISTER_SUBSYSTEM(Time);
	Profiler* profiler = REGISTER_SUBSYSTEM(Profiler);
	Scene* scene = REGISTER_SUBSYSTEM(Scene);
	Graphics* graphics = REGISTER_SUBSYSTEM(Graphics);
	Gizmos* gizmos = REGISTER_SUBSYSTEM(Gizmos, graphics);
	Physics* physics = REGISTER_SUBSYSTEM(Physics, gizmos);
	Input* input = REGISTER_SUBSYSTEM(Input);
	Tags* tags = REGISTER_SUBSYSTEM(Tags);
	LayerMask* layerMask = REGISTER_SUBSYSTEM(LayerMask);

	physics->SetGravity(Vector3(0, -9.8f, 0));

	context_ = new RenderingContext();
	context_->SetTime(time);
	context_->SetScene(scene);
	context_->SetProfiler(profiler);
	context_->SetGraphics(graphics);

	Context::SetCurrent(context_);
	cullingUpdateTask_ = new CullingUpdateTask();

	for (int type = 0; type < (int)SubsystemType::_Count; ++type) {
		subsystems_[type]->Awake();
	}
}

void EngineInternal::Shutdown() {
	for (int type = 0; type < (int)SubsystemType::_Count; ++type) {
		subsystems_[type]->OnDestroy();
	}

	context_->OnDestroy();

	for (int type = (int)SubsystemType::_Count - 1; type >= 0; --type) {
		if (type != (int)SubsystemType::FrameEvents) {
			delete subsystems_[type];
		}
	}

	delete context_;

	threadId_ = std::thread::id();

	Screen::sizeChanged.unsubscribe(this);
}

void EngineInternal::BeginFrame() {
	canvas_->MakeCurrent();
}

void EngineInternal::EndFrame() {
	canvas_->SwapBuffers();
	canvas_->DoneCurrent();
}

Subsystem* EngineInternal::GetSubsystem(SubsystemType type) {
	return subsystems_[(int)type];
}

void EngineInternal::UpdateTimeUniformBuffer() {
	static SharedTimeUniformBuffer p;
	Time* time = (Time*)GetSubsystem(SubsystemType::Time);
	p.time.x = time->GetRealTimeSinceStartup();
	p.time.y = time->GetDeltaTime();
}

void EngineInternal::Update() {
	context_->GetCullingThread()->AddTask(cullingUpdateTask_.get());

	Time* time = (Time*)GetSubsystem(SubsystemType::Time);
	Profiler* profiler = (Profiler*)GetSubsystem(SubsystemType::Profiler);
	uint64 start = Time::GetTimeStamp();

	Screen::Resize(canvas_->GetWidth(), canvas_->GetHeight());

	frameEnterEvent_->raise();

	UpdateTimeUniformBuffer();

	float deltaTime = time->GetDeltaTime();
	context_->Update(deltaTime);

	for (int type = 0; type < (int)SubsystemType::_Count; ++type) {
		subsystems_[type]->Update(deltaTime);
	}

	frameLeaveEvent_->raise();

	profiler->SetRenderingElapsed(
		Time::TimeStampToSeconds(Time::GetTimeStamp() - start)
	);

	context_->GetCullingThread()->Join();
}
