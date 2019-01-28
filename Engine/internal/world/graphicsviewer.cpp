#include "screen.h"
#include "engine.h"
#include "profiler.h"
#include "graphicsviewer.h"
#include "graphicscanvas.h"
#include "engineinternal.h"

#include "memory/memory.h"

enum {
	ViewerStatusUninitialized,
	ViewerStatusRunning,
	ViewerStatusClosed,
};

GraphicsViewer::GraphicsViewer(int argc, char * argv[]) 
	: canvas_(nullptr), status_(ViewerStatusUninitialized) {
}

GraphicsViewer::~GraphicsViewer() {
	Engine::Shutdown();
}

#include "debug/debug.h"

void GraphicsViewer::Run() {
	for (; status_ != ViewerStatusClosed;) {
		if (canvas_ != nullptr) {
			canvas_->MakeCurrent();

			uint64 start = Profiler::GetTimeStamp();
			Update();
			uint64 now = Profiler::GetTimeStamp();
			double qt = Profiler::TimeStampToSeconds(now - start);

			start = now;
			Engine::Update();
			now = Profiler::GetTimeStamp();
			double engine = Profiler::TimeStampToSeconds(now - start);

			start = now;
 			canvas_->SwapBuffers();
 			canvas_->DoneCurrent();
			now = Profiler::GetTimeStamp();
			double swap = Profiler::TimeStampToSeconds(now - start);

			Debug::Output(0, "qt: %.2f, engine: %.2f, swap: %.2f", qt * 1000, engine * 1000, swap * 1000);
		}
	}
}

bool GraphicsViewer::SetCanvas(GraphicsCanvas* value) {
	canvas_ = value;

	if (status_ == ViewerStatusUninitialized) {
		if (Engine::Startup(value->GetWidth(), value->GetHeight())) {
			status_ = ViewerStatusRunning;
		}
	}

	return status_ == ViewerStatusRunning;
}

void GraphicsViewer::OnCanvasSizeChanged(uint width, uint height) {
	Screen::Resize(width, height);
}

void GraphicsViewer::Close() {
	status_ = ViewerStatusClosed;
}
