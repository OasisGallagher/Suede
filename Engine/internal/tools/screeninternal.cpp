#include "screeninternal.h"

#include "debug/debug.h"
#include "memory/memory.h"

#undef _dptr
#define _dptr()	((ScreenInternal*)d_)
Screen::Screen() : Singleton2<Screen>(MEMORY_NEW(ScreenInternal)) {}
uint Screen::GetWidth() { return _dptr()->GetWidth(); }
uint Screen::GetHeight() { return _dptr()->GetHeight(); }
void Screen::AddScreenSizeChangedListener(ScreenSizeChangedListener* listener) { _dptr()->AddScreenSizeChangedListener(listener); }
void Screen::RemoveScreenSizeChangedListener(ScreenSizeChangedListener* listener) { _dptr()->RemoveScreenSizeChangedListener(listener); }
void Screen::Resize(uint width, uint height) { _dptr()->Resize(width, height); }

void ScreenInternal::AddScreenSizeChangedListener(ScreenSizeChangedListener* listener) {
	if (listener == nullptr) {
		Debug::LogError("invalid screen event listener.");
		return;
	}

	if (std::find(listeners_.begin(), listeners_.end(), listener) == listeners_.end()) {
		listeners_.push_back(listener);
	}
}

void ScreenInternal::RemoveScreenSizeChangedListener(ScreenSizeChangedListener* listener) {
	std::vector<ScreenSizeChangedListener*>::iterator pos = std::find(listeners_.begin(), listeners_.end(), listener);
	if (pos != listeners_.end()) {
		listeners_.erase(pos);
	}
}

void ScreenInternal::Resize(uint _width, uint _height) {
	if (width_ == _width && height_ == _height) {
		return;
	}

	width_ = _width;
	height_ = _height;

	for (uint i = 0; i < listeners_.size(); ++i) {
		listeners_[i]->OnScreenSizeChanged(width_, height_);
	}
}
