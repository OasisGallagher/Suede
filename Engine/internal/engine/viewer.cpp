#include "screen.h"
#include "viewer.h"

#include "engine.h"
#include "physics.h"

Viewer::Viewer(int argc, char * argv[]) : status_(Uninitialized) {
	setlocale(LC_ALL, "");
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
