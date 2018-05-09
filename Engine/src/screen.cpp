#include <vector>
#include "screen.h"
#include "debug/debug.h"

static uint width;
static uint height;
std::vector<ScreenSizeChangedListener*> listeners;

uint Screen::GetWidth() {
	return width;
}

uint Screen::GetHeight() {
	return height;
}

void Screen::AddScreenSizeChangedListener(ScreenSizeChangedListener* listener) {
	if (listener == nullptr) {
		Debug::LogError("invalid screen event listener.");
		return;
	}

	if (std::find(listeners.begin(), listeners.end(), listener) == listeners.end()) {
		listeners.push_back(listener);
	}
}

void Screen::RemoveScreenSizeChangedListener(ScreenSizeChangedListener* listener) {
	std::vector<ScreenSizeChangedListener*>::iterator pos = std::find(listeners.begin(), listeners.end(), listener);
	if (pos != listeners.end()) {
		listeners.erase(pos);
	}
}

void Screen::Set(uint _width, uint _height) {
	if (width == _width && height == _height) {
		return;
	}

	width = _width;
	height = _height;

	for (uint i = 0; i < listeners.size(); ++i) {
		listeners[i]->OnScreenSizeChanged(width, height);
	}
}
