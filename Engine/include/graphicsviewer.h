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
	void OnCanvasSizeChanged(uint width, uint height);

protected:
	void Close();

protected:
	virtual void Update() = 0;

private:
	int status_;
	GraphicsCanvas* canvas_;
};
