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
	RenderMesh(entity->GetMesh());
}

void RendererInternal::UpdateMaterial(Entity entity) {
}

void RendererInternal::RenderMesh(Mesh mesh) {
	int subMeshCount = mesh->GetSubMeshCount();
	int materialCount = GetMaterialCount();

	if (materialCount != subMeshCount) {
		Debug::LogError("material count mismatch with sub mesh count");
		return;
	}

	for (int i = 0; i < subMeshCount; ++i) {
		Material material = GetMaterial(i);
		int pass = material->GetPass();
		if (pass >= 0 && material->IsPassEnabled(pass)) {
			RenderSubMesh(mesh, i, material, pass);
		}
		else {
			for (pass = 0; pass < material->GetPassCount(); ++pass) {
				if (material->IsPassEnabled(pass)) {
					RenderSubMesh(mesh, i, material, pass);
				}
			}
		}
	}
}

void RendererInternal::RenderMesh(Mesh mesh, Material material, int pass) {
	for (int i = 0; i < mesh->GetSubMeshCount(); ++i) {
		AddToPipeline(mesh->GetSubMesh(i), material, pass);
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

void RendererInternal::AddToPipeline(SubMesh subMesh, Material material, int pass) {
	Renderable* item = Pipeline::GetCurrent()->CreateRenderable();
	item->pass = pass;
	item->instance = 0;
	item->material = material;
	item->subMesh = subMesh;
	Framebuffer::GetCurrentWrite()->SaveState(item->state);
}

void RendererInternal::RenderSubMesh(Mesh mesh, int subMeshIndex, Material material, int pass) {
	AddToPipeline(mesh->GetSubMesh(subMeshIndex), material, pass);
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

void ParticleRendererInternal::AddToPipeline(SubMesh subMesh, Material material, int pass) {
	if (particleCount_ == 0) { return; }
	Renderable* item = Pipeline::GetCurrent()->CreateRenderable();
	item->pass = pass;
	item->material = material;
	item->subMesh = subMesh;
	item->instance = particleCount_;
	Framebuffer::GetCurrentWrite()->SaveState(item->state);
}
