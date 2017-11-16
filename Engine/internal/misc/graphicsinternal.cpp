#include "variables.h"
#include "tools/math2.h"
#include "graphicsinternal.h"
#include "internal/memory/memory.h"
#include "internal/world/worldinternal.h"
#include "internal/resources/resources.h"
#include "internal/base/surfaceinternal.h"
#include "internal/base/rendererinternal.h"

Graphics graphicsInstance(MEMORY_CREATE(GraphicsInternal));

void GraphicsInternal::Blit(RenderTexture src, RenderTexture dest, Material material) {
	Surface surface = Resources::GetPrimitive(PrimitiveTypeQuad);
	Renderer renderer = CREATE_OBJECT(SurfaceRenderer);

	material->SetRenderState(Cull, Off);
	material->SetRenderState(DepthWrite, Off);
	material->SetRenderState(DepthTest, Always);
	material->SetTexture(Variables::mainTexture, src);

	material->Bind();
	renderer->RenderSurface(surface);
	material->Unbind();
}
