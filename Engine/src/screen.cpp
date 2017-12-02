#include "screen.h"

static uint width;
static uint height;

uint Screen::GetWidth() {
	return width;
}

uint Screen::GetHeight() {
	return height;
}

void Screen::Set(uint w, uint h) {
	width = w;
	height = h;
}
