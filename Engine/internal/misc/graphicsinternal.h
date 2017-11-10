#include "surface.h"
#include "graphics.h"

extern Graphics graphicsInstance;
class GraphicsInternal : public IGraphics {
public:
	virtual void Blit(RenderTexture src, RenderTexture dest, Material material);

private:
	Surface GetQuadSurface();
};
