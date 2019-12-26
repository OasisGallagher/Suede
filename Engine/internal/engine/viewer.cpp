#include "screen.h"
#include "engine.h"
#include "physics.h"
#include "profiler.h"
#include "viewer.h"
#include "glcanvas.h"

#include "debug/debug.h"

static void OnTerminate() {
	Debug::Break();
}

Viewer::Viewer(int argc, char * argv[]) : status_(Uninitialized) {
	setlocale(LC_ALL, "");
	std::set_terminate(OnTerminate);
}

void Viewer::Run() {
	for (; status_ == Running;) {
		Engine::BeginFrame();

		Update();

		if (!IsClosed()) {
			Engine::Update();
		}

		Engine::EndFrame();
	}
}

bool Viewer::StartupEngine(GLCanvas* value) {
	SUEDE_ASSERT(value != nullptr);
	SUEDE_ASSERT(status_ == Uninitialized);

	Engine::Startup(value);
	status_ = Running;

	return true;
}

void Viewer::Close() {
	Engine::Shutdown();
	status_ = Closed;
}
