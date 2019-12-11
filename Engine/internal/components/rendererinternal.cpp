#include "rendererinternal.h"

#include <algorithm>

#include "builtinproperties.h"
#include "internal/base/renderdefines.h"
#include "internal/rendering/pipeline.h"

Renderer::Renderer(void* d) : Component(d) {}
void Renderer::AddMaterial(Material* material) { _suede_dptr()->AddMaterial(material); }
Material* Renderer::GetMaterial(uint index) { return _suede_dptr()->GetMaterial(index); }
void Renderer::SetMaterial(uint index, Material* value) { _suede_dptr()->SetMaterial(index, value); }
void Renderer::RemoveMaterial(Material* material) { _suede_dptr()->RemoveMaterial(material); }
void Renderer::RemoveMaterialAt(uint index) { _suede_dptr()->RemoveMaterialAt(index); }
uint Renderer::GetMaterialCount() { return _suede_dptr()->GetMaterialCount(); }
void Renderer::UpdateMaterialProperties() { _suede_dptr()->UpdateMaterialProperties(); }

MeshRenderer::MeshRenderer() : Renderer(MEMORY_NEW(MeshRendererInternal)) {}

ParticleRenderer::ParticleRenderer() : Renderer(MEMORY_NEW(ParticleRendererInternal)) {}

SkinnedMeshRenderer::SkinnedMeshRenderer() : Renderer(MEMORY_NEW(SkinnedMeshRendererInternal)) {}
void SkinnedMeshRenderer::SetSkeleton(Skeleton* value) { _suede_dptr()->SetSkeleton(value); }

SUEDE_DEFINE_COMPONENT_INTERNAL(Renderer, Component)
SUEDE_DEFINE_COMPONENT_INTERNAL(MeshRenderer, Renderer)
SUEDE_DEFINE_COMPONENT_INTERNAL(ParticleRenderer, Renderer)
SUEDE_DEFINE_COMPONENT_INTERNAL(SkinnedMeshRenderer, Renderer)

RendererInternal::RendererInternal(ObjectType type) : ComponentInternal(type) {
}

RendererInternal::~RendererInternal() {
}

void RendererInternal::RemoveMaterial(Material* material) {
	materials_.erase(
		std::remove(materials_.begin(), materials_.end(), material),
		materials_.end()
	);
}

void RendererInternal::RemoveMaterialAt(uint index) {
	SUEDE_VERIFY_INDEX(index, materials_.size(), SUEDE_NOARG);
	materials_.erase(materials_.begin() + index);
}

void SkinnedMeshRendererInternal::UpdateMaterialProperties() {
	for (int i = 0; i < GetMaterialCount(); ++i) {
		Material* material = GetMaterial(i);
		material->SetMatrix4Array(BuiltinProperties::BoneToRootMatrices, skeleton_->GetBoneToRootMatrices(), MAX_BONE_COUNT);
	}
}

ParticleRendererInternal::ParticleRendererInternal()
	: RendererInternal(ObjectType::ParticleRenderer) {
}
