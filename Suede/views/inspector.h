#pragma once
#include "window.h"

class Inspector : public Window {
	Q_OBJECT

public:
	static Inspector* get();

private:
	Inspector() {}
	virtual void initialize();
};
