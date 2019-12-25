#include "screen.h"
#include "world.h"
#include "physics.h"
#include "profiler.h"
#include "graphicsviewer.h"
#include "graphicscanvas.h"

#include "debug/debug.h"

enum {
	ViewerStatusUninitialized,
	ViewerStatusRunning,
	ViewerStatusClosed,
};

static void OnTerminate() {
	Debug::Break();
}

GraphicsViewer::GraphicsViewer(int argc, char * argv[])
	: canvas_(nullptr), status_(ViewerStatusUninitialized) {
	setlocale(LC_ALL, "");
	std::set_terminate(OnTerminate);
}

void GraphicsViewer::Run() {
	for (; status_ != ViewerStatusClosed;) {
		if (canvas_ == nullptr) {
			continue;
		}

		canvas_->MakeCurrent();

		Update();

		if (status_ != ViewerStatusClosed) {
			World::Update();
		}

		canvas_->SwapBuffers();
		canvas_->DoneCurrent();
	}
}

bool GraphicsViewer::SetCanvas(GraphicsCanvas* value) {
	canvas_ = value;

	if (canvas_ != nullptr && status_ == ViewerStatusUninitialized) {
		Screen::Resize(value->GetWidth(), value->GetHeight());

		World::Initialize();
		Physics::SetGravity(Vector3(0, -9.8f, 0));

		status_ = ViewerStatusRunning;
	}

	return status_ == ViewerStatusRunning;
}

void GraphicsViewer::Close() {
	World::Finalize();
	status_ = ViewerStatusClosed;
}

void GraphicsViewer::Update() {
	Screen::Resize(canvas_->GetWidth(), canvas_->GetHeight());
}
