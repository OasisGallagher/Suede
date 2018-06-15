#pragma once
#include "types.h"

class GraphicsCanvas;
class SUEDE_API GraphicsViewer {
public:
	GraphicsViewer(int argc, char *argv[]);
	virtual ~GraphicsViewer();

	static void _GraphicsViewer_();

public:
	void Run();
	void SetCanvas(GraphicsCanvas* value);
	void OnCanvasSizeChanged(uint width, uint height);

protected:
	virtual void Update();
};
