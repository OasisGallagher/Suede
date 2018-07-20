#include "screen.h"
#include "debug/debug.h"

void Screen::AddScreenSizeChangedListener(ScreenSizeChangedListener* listener) {
	if (listener == nullptr) {
		Debug::LogError("invalid screen event listener.");
		return;
	}

	if (std::find(listeners_.begin(), listeners_.end(), listener) == listeners_.end()) {
		listeners_.push_back(listener);
	}
}

void Screen::RemoveScreenSizeChangedListener(ScreenSizeChangedListener* listener) {
	std::vector<ScreenSizeChangedListener*>::iterator pos = std::find(listeners_.begin(), listeners_.end(), listener);
	if (pos != listeners_.end()) {
		listeners_.erase(pos);
	}
}

void Screen::Set(uint _width, uint _height) {
	if (width_ == _width && height_ == _height) {
		return;
	}

	width_ = _width;
	height_ = _height;

	for (uint i = 0; i < listeners_.size(); ++i) {
		listeners_[i]->OnScreenSizeChanged(width_, height_);
	}
}
