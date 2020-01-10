#pragma once
#include "types.h"

class GLCanvas;
class SUEDE_API Viewer {
	enum {
		Uninitialized,
		Running,
		Closed,
	};

public:
	Viewer(int argc, char *argv[]);
	virtual ~Viewer() {}

public:
	void Run();
	bool IsClosed() { return loaded_ == Closed; }
	bool StartupEngine(GLCanvas* value);

protected:
	void Close();

protected:
	virtual void Update() {}

private:
	int loaded_;
};
