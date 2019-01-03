#include "screeninternal.h"

#include "debug/debug.h"
#include "memory/memory.h"

Screen::Screen() : singleton2<Screen>(MEMORY_NEW(ScreenInternal), Memory::DeleteRaw<ScreenInternal>) {}
uint Screen::GetWidth() { return _suede_dinstance()->GetWidth(); }
uint Screen::GetHeight() { return _suede_dinstance()->GetHeight(); }
void Screen::AddScreenSizeChangedListener(ScreenSizeChangedListener* listener) { _suede_dinstance()->AddScreenSizeChangedListener(listener); }
void Screen::RemoveScreenSizeChangedListener(ScreenSizeChangedListener* listener) { _suede_dinstance()->RemoveScreenSizeChangedListener(listener); }
void Screen::Resize(uint width, uint height) { _suede_dinstance()->Resize(width, height); }

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

void ScreenInternal::Resize(uint width, uint height) {
	if (width_ == width && height_ == height) {
		return;
	}

	width_ = width;
	height_ = height;

	for (uint i = 0; i < listeners_.size(); ++i) {
		listeners_[i]->OnScreenSizeChanged(width_, height_);
	}
}
