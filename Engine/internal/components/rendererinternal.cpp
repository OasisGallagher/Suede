#include "rendererinternal.h"

#include <algorithm>

#include "builtinproperties.h"
#include "internal/base/renderdefines.h"
#include "internal/rendering/pipeline.h"

IRenderer::IRenderer(void* d) : IComponent(d) {}
void IRenderer::AddMaterial(Material material) { _suede_dptr()->AddMaterial(material); }
Material IRenderer::GetMaterial(uint index) { return _suede_dptr()->GetMaterial(index); }
IRenderer::Enumerable IRenderer::GetMaterials() { return _suede_dptr()->GetMaterials(); }
void IRenderer::SetMaterial(uint index, Material value) { _suede_dptr()->SetMaterial(index, value); }
void IRenderer::RemoveMaterial(Material material) { _suede_dptr()->RemoveMaterial(material); }
void IRenderer::RemoveMaterialAt(uint index) { _suede_dptr()->RemoveMaterialAt(index); }
uint IRenderer::GetMaterialCount() { return _suede_dptr()->GetMaterialCount(); }
void IRenderer::UpdateMaterialProperties() { _suede_dptr()->UpdateMaterialProperties(); }

IMeshRenderer::IMeshRenderer() : IRenderer(MEMORY_NEW(MeshRendererInternal)) {}

IParticleRenderer::IParticleRenderer() : IRenderer(MEMORY_NEW(ParticleRendererInternal)) {}

ISkinnedMeshRenderer::ISkinnedMeshRenderer() : IRenderer(MEMORY_NEW(ParticleRendererInternal)) {}
void ISkinnedMeshRenderer::SetSkeleton(Skeleton value) { _suede_dptr()->SetSkeleton(value); }

SUEDE_DEFINE_COMPONENT(IRenderer, IComponent)
SUEDE_DEFINE_COMPONENT(IMeshRenderer, IRenderer)
SUEDE_DEFINE_COMPONENT(IParticleRenderer, IRenderer)
SUEDE_DEFINE_COMPONENT(ISkinnedMeshRenderer, IRenderer)

RendererInternal::RendererInternal(ObjectType type) : ComponentInternal(type) {
}

RendererInternal::~RendererInternal() {
}

void RendererInternal::RemoveMaterial(Material material) {
	std::vector<Material>::iterator pos = std::remove(materials_.begin(), materials_.end(), material);
	materials_.erase(pos, materials_.end());
}

void RendererInternal::RemoveMaterialAt(uint index) {
	VERIFY_INDEX(index, materials_.size(), NOARG);
	materials_.erase(materials_.begin() + index);
}

void SkinnedMeshRendererInternal::UpdateMaterialProperties() {
	for (Material material : GetMaterials()) {
		material->SetMatrix4Array(BuiltinProperties::BoneToRootMatrices, skeleton_->GetBoneToRootMatrices(), MAX_BONE_COUNT);
	}
}

ParticleRendererInternal::ParticleRendererInternal()
	: RendererInternal(ObjectType::ParticleRenderer) {
}
