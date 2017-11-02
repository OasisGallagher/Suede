#include "graphicsinternal.h"
#include "internal/memory/memory.h"
#include "internal/file/modelimporter.h"
#include "internal/world/worldinternal.h"

Graphics graphicsInstance(Memory::Create<GraphicsInternal>());

void GraphicsInternal::Blit(RenderTexture src, RenderTexture dest, Renderer renderer) {
	ModelImporter importer;
	importer.Import("buildin/models/quad.obj", ModelImporter::MaskNone);
	Surface surface = importer.GetSurface();

	for (int i = 0; i < renderer->GetMaterialCount(); ++i) {
		Material material = renderer->GetMaterial(i);
		Mesh mesh = surface->GetMesh(0);
		MaterialTextures& textures = mesh->GetMaterialTextures();
		textures.albedo = src;
	}

	renderer->SetRenderState(Cull, Off);
	renderer->SetRenderState(DepthWrite, Off);
	renderer->SetRenderState(DepthTest, Always);

	renderer->RenderSurface(surface);
}
