#include "variables.h"
#include "tools/math2.h"
#include "graphicsinternal.h"
#include "internal/memory/memory.h"
#include "internal/base/meshinternal.h"
#include "internal/world/worldinternal.h"
#include "internal/resources/resources.h"
#include "internal/base/rendererinternal.h"

Graphics graphicsInstance(MEMORY_CREATE(GraphicsInternal));

void GraphicsInternal::Blit(RenderTexture src, RenderTexture dest, Material material) {
	Mesh mesh = Resources::GetPrimitive(PrimitiveTypeQuad);
	Renderer renderer = CREATE_OBJECT(MeshRenderer);

	material->SetRenderState(Cull, Off);
	material->SetRenderState(DepthWrite, Off);
	material->SetRenderState(DepthTest, Always);
	material->SetTexture(Variables::mainTexture, src);

	renderer->RenderMesh(mesh, material);
}
