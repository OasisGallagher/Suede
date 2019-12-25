#pragma once
#include "types.h"

class GraphicsCanvas;
class SUEDE_API GraphicsViewer {
public:
	GraphicsViewer(int argc, char *argv[]);
	virtual ~GraphicsViewer();

public:
	void Run();
	bool SetCanvas(GraphicsCanvas* value);
	GraphicsCanvas* GetCanvas() { return canvas_; }

protected:
	void Close();

protected:
	virtual void Update();

private:
	int status_;
	GraphicsCanvas* canvas_;
};
