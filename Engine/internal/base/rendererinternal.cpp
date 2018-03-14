#include <algorithm>

#include "variables.h"
#include "framebuffer.h"
#include "tools/math2.h"
#include "debug/debug.h"
#include "rendererinternal.h"
#include "internal/rendering/pipeline.h"

RendererInternal::RendererInternal(ObjectType type) : ObjectInternal(type) {	
}

RendererInternal::~RendererInternal() {
}

void RendererInternal::RenderEntity(Entity entity) {
	UpdateMaterial(entity);
	RenderMesh(entity);
}

void RendererInternal::UpdateMaterial(Entity entity) {
}

void RendererInternal::RenderMesh(Entity entity) {
	int subMeshCount = entity->GetMesh()->GetSubMeshCount();
	int materialCount = GetMaterialCount();

	if (materialCount != subMeshCount) {
		Debug::LogError("material count mismatch with sub mesh count");
		return;
	}

	for (int i = 0; i < subMeshCount; ++i) {
		Material material = GetMaterial(i);
		int pass = material->GetPass();
		if (pass >= 0 && material->IsPassEnabled(pass)) {
			RenderSubMesh(entity, i, material, pass);
		}
		else {
			for (pass = 0; pass < material->GetPassCount(); ++pass) {
				if (material->IsPassEnabled(pass)) {
					RenderSubMesh(entity, i, material, pass);
				}
			}
		}
	}
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

void RendererInternal::AddToPipeline(Entity entity, uint subMeshIndex, Material material, int pass) {
	FramebufferState state;
	Pipeline::GetFramebuffer()->SaveState(state);
	Pipeline::GetCurrent()->AddRenderable(entity->GetMesh(), subMeshIndex, material, pass, state, entity->GetTransform()->GetLocalToWorldMatrix());
}

void RendererInternal::RenderSubMesh(Entity entity, int subMeshIndex, Material material, int pass) {
	AddToPipeline(entity, subMeshIndex, material, pass);
}

void SkinnedMeshRendererInternal::UpdateMaterial(Entity entity) {
	for (int i = 0; i < GetMaterialCount(); ++i) {
		GetMaterial(i)->SetMatrix4Array(Variables::boneToRootSpaceMatrices, skeleton_->GetBoneToRootSpaceMatrices(), C_MAX_BONE_COUNT);
	}

	RendererInternal::UpdateMaterial(entity);
}

ParticleRendererInternal::ParticleRendererInternal()
	: RendererInternal(ObjectTypeParticleRenderer), particleCount_(0) {
}

void ParticleRendererInternal::AddMaterial(Material material) {
	RendererInternal::AddMaterial(material);
}

void ParticleRendererInternal::RenderEntity(Entity entity) {
	ParticleSystem particleSystem = dsp_cast<ParticleSystem>(entity);
	if (!particleSystem) {
		Debug::LogError("invalid particle system");
		return;
	}

	particleCount_ = particleSystem->GetParticlesCount();

	RendererInternal::RenderEntity(entity);
}

void ParticleRendererInternal::AddToPipeline(Entity entity, uint subMeshIndex, Material material, int pass) {
	if (particleCount_ != 0) {
		FramebufferState state;
		Pipeline::GetFramebuffer()->SaveState(state);
		Pipeline::GetCurrent()->AddRenderable(entity->GetMesh(), subMeshIndex, material, pass, state, entity->GetTransform()->GetLocalToWorldMatrix(), particleCount_);
	}
}
