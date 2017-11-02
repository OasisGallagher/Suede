#include "screen.h"

extern Screen screenInstance;

class ScreenInternal : public IScreen {
public:
	virtual int GetContextWidth() { return width_; }
	virtual int GetContextHeight() { return height_; }

	virtual void SetContentSize(int w, int h) { width_ = w, height_ = h; }

private:
	int width_;
	int height_;
};
