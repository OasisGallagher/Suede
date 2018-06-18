#include "screen.h"
#include "engine.h"
#include "graphicsviewer.h"
#include "graphicscanvas.h"

enum {
	ViewerStatusUninitialized,
	ViewerStatusRunning,
	ViewerStatusClosed,
};

GraphicsViewer::GraphicsViewer(int argc, char * argv[]) 
	: canvas_(nullptr), status_(ViewerStatusUninitialized) {
}

GraphicsViewer::~GraphicsViewer() {
	Engine::Release();
}

void GraphicsViewer::Run() {
	for (; status_ != ViewerStatusClosed;) {
		Update();

		if (canvas_ != nullptr) {
			canvas_->MakeCurrent();
			Engine::Update();

			canvas_->SwapBuffers();
			canvas_->DoneCurrent();
		}
	}
}

void GraphicsViewer::SetCanvas(GraphicsCanvas* value) {
	if (status_ == ViewerStatusUninitialized) {
		Engine::Initialize();
		status_ = ViewerStatusRunning;
	}

	canvas_ = value;
	Screen::Set(value->GetWidth(), value->GetHeight());
}

void GraphicsViewer::OnCanvasSizeChanged(uint width, uint height) {
	Screen::Set(width, height);
}

void GraphicsViewer::Close() {
	status_ = ViewerStatusClosed;
}

void GraphicsViewer::Update() {
}
