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

EngineInternal::EngineInternal() {
	static FrameEvents eventSystem;
	frameEnterEvent_ = &eventSystem.frameEnter;
	frameLeaveEvent_ = &eventSystem.frameLeave;

	subsystems_[(int)SubsystemType::FrameEvents] = &eventSystem;
}

void EngineInternal::Startup(GLCanvas* canvas) {
	canvas_ = canvas;
	Screen::Resize(canvas->GetWidth(), canvas->GetHeight());
	threadId_ = std::this_thread::get_id();

	Time* time = new Time();
	subsystems_[(int)SubsystemType::Time] = time;

	Profiler* profiler = new Profiler();
	subsystems_[(int)SubsystemType::Profiler] = profiler;

	Scene* scene = new Scene();
	subsystems_[(int)SubsystemType::Scene] = scene;

	Graphics* graphics = new Graphics();
	subsystems_[(int)SubsystemType::Graphics] = graphics;

	Gizmos* gizmos = new Gizmos(graphics);
	subsystems_[(int)SubsystemType::Gizmos] = gizmos;

	Physics* physics = new Physics(gizmos);
	subsystems_[(int)SubsystemType::Physics] = physics;
	physics->SetGravity(Vector3(0, -9.8f, 0));

	subsystems_[(int)SubsystemType::Input] = new Input();
	subsystems_[(int)SubsystemType::Tags] = new Tags();

	context_ = new RenderingContext();
	context_->SetTime(time);
	context_->SetScene(scene);
	context_->SetProfiler(profiler);
	context_->SetGraphics(graphics);

	Context::SetCurrent(context_);

	for (int type = 0; type < (int)SubsystemType::_Count; ++type) {
		subsystems_[type]->Awake();
	}
}

void EngineInternal::Shutdown() {
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
	Time* time = (Time*)GetSubsystem(SubsystemType::Time);
	Profiler* profiler = (Profiler*)GetSubsystem(SubsystemType::Profiler);
	uint64 start = Time::GetTimeStamp();

	Screen::Resize(canvas_->GetWidth(), canvas_->GetHeight());

	frameEnterEvent_->raise();

	UpdateTimeUniformBuffer();

	context_->Update(time->GetDeltaTime());

	for (int type = 0; type < (int)SubsystemType::_Count; ++type) {
		subsystems_[type]->Update(time->GetDeltaTime());
	}

	frameLeaveEvent_->raise();

	profiler->SetRenderingElapsed(
		Time::TimeStampToSeconds(Time::GetTimeStamp() - start)
	);
}
