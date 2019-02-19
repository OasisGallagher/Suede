#include "screen.h"
#include "engine.h"
#include "profiler.h"
#include "graphicsviewer.h"
#include "graphicscanvas.h"
#include "engineinternal.h"

#include "memory/memory.h"

GraphicsViewer::GraphicsViewer(int argc, char * argv[]) 
	: canvas_(nullptr), status_(Uninitialized) {
}

GraphicsViewer::~GraphicsViewer() {
	Engine::Shutdown();
}

#include "debug/debug.h"

void GraphicsViewer::Run() {
	for (; status_ != Closed;) {
		if (canvas_ != nullptr) {
			canvas_->MakeCurrent();

			PROFILER_RECORD(qt,
				Update()
			);

			PROFILER_RECORD(engine,
				Engine::Update()
			);

			PROFILER_RECORD(swap,
 				canvas_->SwapBuffers();
 				canvas_->DoneCurrent();
			);

			Debug::Output(0, "qt: %.2f ms, engine: %.2f ms, swap: %.2f ms", qt * 1000, engine * 1000, swap * 1000);
			Debug::Output(0, "=========================================================");
		}
	}
}

bool GraphicsViewer::SetCanvas(GraphicsCanvas* value) {
	canvas_ = value;

	if (status_ == Uninitialized) {
		if (Engine::Startup(value->GetWidth(), value->GetHeight())) {
			status_ = Running;
		}
	}

	return status_ == Running;
}

void GraphicsViewer::OnCanvasSizeChanged(uint width, uint height) {
	Screen::Resize(width, height);
}

void GraphicsViewer::Close() {
	status_ = Closed;
}
