#include "variables.h"
#include "graphicsinternal.h"
#include "internal/memory/memory.h"
#include "internal/file/assetimporter.h"
#include "internal/world/worldinternal.h"

Graphics graphicsInstance(Memory::Create<GraphicsInternal>());

void GraphicsInternal::Blit(RenderTexture src, RenderTexture dest, Renderer renderer) {
	static Surface surface;
	if (!surface) {
		AssetImporter importer;
		surface = importer.ImportSurface("buildin/models/quad.obj");
	}

	for (int i = 0; i < renderer->GetMaterialCount(); ++i) {
		Material material = renderer->GetMaterial(i);
		material->SetRenderState(Cull, Off);
		material->SetRenderState(DepthWrite, Off);
		material->SetRenderState(DepthTest, Always);
		material->SetTexture(Variables::mainTexture, src);
	}

	renderer->RenderSurface(surface);
}
