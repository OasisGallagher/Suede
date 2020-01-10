#include "screen.h"
#include "viewer.h"

#include "engine.h"
#include "physics.h"

Viewer::Viewer(int argc, char * argv[]) : loaded_(Uninitialized) {
	setlocale(LC_ALL, "");
}

void Viewer::Run() {
	for (; loaded_ == Running;) {
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
	SUEDE_ASSERT(loaded_ == Uninitialized);

	Engine::Startup(value);
	loaded_ = Running;

	return true;
}

void Viewer::Close() {
	Engine::Shutdown();
	loaded_ = Closed;
}
