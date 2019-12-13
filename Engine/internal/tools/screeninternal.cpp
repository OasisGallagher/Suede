#include "screeninternal.h"

#include "debug/debug.h"
#include "memory/refptr.h"

event<uint, uint> Screen::sizeChanged;
Screen::Screen() : Singleton2<Screen>(new ScreenInternal, t_delete<ScreenInternal>) {}
uint Screen::GetWidth() { return _suede_dinstance()->GetWidth(); }
uint Screen::GetHeight() { return _suede_dinstance()->GetHeight(); }
void Screen::Resize(uint width, uint height) { _suede_dinstance()->Resize(width, height); }

void ScreenInternal::Resize(uint width, uint height) {
	if (width_ == width && height_ == height) {
		return;
	}

	width_ = width;
	height_ = height;

	Screen::sizeChanged.fire(width_, height_);
}
