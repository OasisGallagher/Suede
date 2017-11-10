#include "variables.h"
#include "graphicsinternal.h"
#include "internal/memory/memory.h"
#include "internal/file/assetimporter.h"
#include "internal/world/worldinternal.h"
#include "internal/base/rendererinternal.h"

Graphics graphicsInstance(Memory::Create<GraphicsInternal>());

void GraphicsInternal::Blit(RenderTexture src, RenderTexture dest, Material material) {
	static Surface surface;
	if (!surface) {
		AssetImporter importer;
		surface = importer.ImportSurface("buildin/models/quad.obj");
	}

	Renderer renderer = CREATE_OBJECT(SurfaceRenderer);

	material->SetRenderState(Cull, Off);
	material->SetRenderState(DepthWrite, Off);
	material->SetRenderState(DepthTest, Always);
	material->SetTexture(Variables::mainTexture, src);

	material->Bind();
	renderer->RenderSurface(surface);
	material->Unbind();
}
