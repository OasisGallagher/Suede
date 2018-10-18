#include "screen.h"
#include "engine.h"
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
	Engine::instance()->Shutdown();
}

void GraphicsViewer::Run() {
	for (; status_ != ViewerStatusClosed;) {
		if (canvas_ != nullptr) {
			canvas_->MakeCurrent();

			Update();
			Engine::instance()->Update();

 			canvas_->SwapBuffers();
 			canvas_->DoneCurrent();
		}
	}
}

bool GraphicsViewer::SetCanvas(GraphicsCanvas* value) {
	canvas_ = value;

	if (status_ == ViewerStatusUninitialized) {
		Engine::implement(MEMORY_NEW(EngineInternal));
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
