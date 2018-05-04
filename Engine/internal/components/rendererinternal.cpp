#include <algorithm>

#include "variables.h"
#include "rendererinternal.h"
#include "internal/rendering/pipeline.h"

RendererInternal::RendererInternal(ObjectType type) : ComponentInternal(type) {
}

RendererInternal::~RendererInternal() {
}

void RendererInternal::RemoveMaterial(Material material) {
	std::vector<Material>::iterator pos = std::remove(materials_.begin(), materials_.end(), material);
	materials_.erase(pos, materials_.end());
}

void RendererInternal::RemoveMaterialAt(uint index) {
	if (index >= materials_.size()) {
		Debug::LogError("index out of range.");
		return;
	}

	materials_.erase(materials_.begin() + index);
}

void SkinnedMeshRendererInternal::UpdateMaterialProperties() {
	for (int i = 0; i < GetMaterialCount(); ++i) {
		GetMaterial(i)->SetMatrix4Array(Variables::boneToRootMatrices, skeleton_->GetBoneToRootMatrices(), C_MAX_BONE_COUNT);
	}
}

ParticleRendererInternal::ParticleRendererInternal()
	: RendererInternal(ObjectTypeParticleRenderer) {
}
