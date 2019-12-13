#include "screen.h"

event<uint, uint> Screen::sizeChanged;

static uint width = 0, height = 0;

uint Screen::GetWidth() { return width; }
uint Screen::GetHeight() { return height; }

void Screen::Resize(uint w, uint h) {
	if (w != width || h != height) {
		width = w;
		height = h;
		Screen::sizeChanged.raise(width, height);
	}
}
