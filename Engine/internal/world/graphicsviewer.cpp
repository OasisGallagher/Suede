#include "screen.h"
#include "engine.h"
#include "graphicsviewer.h"
#include "graphicscanvas.h"
#include "engineinternal.h"

enum {
	ViewerStatusUninitialized,
	ViewerStatusRunning,
	ViewerStatusClosed,
};

GraphicsViewer::GraphicsViewer(int argc, char * argv[]) 
	: canvas_(nullptr), status_(ViewerStatusUninitialized) {
}

GraphicsViewer::~GraphicsViewer() {
	Engine::instance()->Shutdown();
}

// #include <Windows.h>
// #include "debug/debug.h"
void GraphicsViewer::Run() {
	for (; status_ != ViewerStatusClosed;) {
// 		const int FRAMES_PER_SECOND = 60;
// 		const int SKIP_TICKS = 1000 / FRAMES_PER_SECOND;
// 		int next_game_tick = GetTickCount();

		if (canvas_ != nullptr) {
			canvas_->MakeCurrent();

			Update();
			Engine::instance()->Update();

			canvas_->SwapBuffers();
			canvas_->DoneCurrent();
		}

// 		next_game_tick += SKIP_TICKS;
// 		int sleep_time = next_game_tick - GetTickCount();
// 		if (sleep_time >= 0) {
// 			Sleep(sleep_time);
// 		}
// 		else {
// 			Debug::Output("");
// 		}
	}
}

bool GraphicsViewer::SetCanvas(GraphicsCanvas* value) {
	canvas_ = value;

	if (status_ == ViewerStatusUninitialized) {
		Engine::implement(new EngineInternal);
		if (Engine::instance()->Startup(value->GetWidth(), value->GetHeight())) {
			status_ = ViewerStatusRunning;
		}
	}

	return status_ == ViewerStatusRunning;
}

void GraphicsViewer::OnCanvasSizeChanged(uint width, uint height) {
	if (Screen::instance()) {
		Screen::instance()->Resize(width, height);
	}
}

void GraphicsViewer::Close() {
	status_ = ViewerStatusClosed;
}
