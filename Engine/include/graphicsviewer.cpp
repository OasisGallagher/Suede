#include "screen.h"
#include "engine.h"
#include "graphicsviewer.h"
#include "graphicscanvas.h"

GraphicsViewer::GraphicsViewer(int argc, char * argv[]) {
}

void GraphicsViewer::_GraphicsViewer_() {
	Engine::Initialize();
}

GraphicsViewer::~GraphicsViewer() {
	Engine::Release();
}

void GraphicsViewer::Run() {
	for (;;) {
		Update();
	}
}

void GraphicsViewer::SetCanvas(GraphicsCanvas* value) {
	Engine::SetCanvas(value);
	Screen::Set(value->GetWidth(), value->GetHeight());
}

void GraphicsViewer::OnCanvasSizeChanged(uint width, uint height) {
	Screen::Set(width, height);
}

void GraphicsViewer::Update() {
}
