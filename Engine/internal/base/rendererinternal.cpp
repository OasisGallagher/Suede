#include <algorithm>

#include "time2.h"
#include "variables.h"
#include "tools/math2.h"
#include "debug/debug.h"
#include "rendererinternal.h"

RendererInternal::RendererInternal(ObjectType type) : ObjectInternal(type), queue_(RenderQueueGeometry) {	
}

RendererInternal::~RendererInternal() {
}

void RendererInternal::RenderEntity(Entity entity) {
	UpdateMaterial(entity);

	Mesh mesh = entity->GetMesh();
	if (GetMaterialCount() == 1) {
		RenderMesh(mesh, GetMaterial(0));
	}
	else {
		RenderMesh(entity->GetMesh());
	}
}

GLenum RendererInternal::TopologyToGLEnum(MeshTopology topology) {
	if (topology != MeshTopologyTriangles && topology != MeshTopologyTriangleStripes) {
		Debug::LogError("invalid mesh topology");
		return 0;
	}

	if (topology == MeshTopologyTriangles) { return GL_TRIANGLES; }
	return GL_TRIANGLE_STRIP;
}

void RendererInternal::UpdateMaterial(Entity entity) {
	int materialCount = GetMaterialCount();
	for (int i = 0; i < materialCount; ++i) {
		Material material = GetMaterial(i);
		material->SetFloat(Variables::time, Time::GetRealTimeSinceStartup());
		material->SetFloat(Variables::deltaTime, Time::GetDeltaTime());

		glm::mat4 localToWorldMatrix = entity->GetLocalToWorldMatrix();
		material->SetMatrix4(Variables::localToWorldSpaceMatrix, localToWorldMatrix);
	}
}

void RendererInternal::RenderMesh(Mesh mesh) {
	int subMeshCount = mesh->GetSubMeshCount();
	int materialCount = GetMaterialCount();

	if (materialCount != subMeshCount) {
		Debug::LogError("material count mismatch with sub mesh count");
		return;
	}

	mesh->Bind();

	for (int i = 0; i < subMeshCount; ++i) {
		Material material = GetMaterial(i);
		int pass = material->GetPass();
		if (pass >= 0 && material->IsPassEnabled(pass)) {
			RenderSubMesh(mesh, i, material, pass);
		}
		else {
			for (int p = 0; p < material->GetPassCount(); ++p) {
				if (material->IsPassEnabled(p)) {
					RenderSubMesh(mesh, i, material, p);
				}
			}
		}
	}

	mesh->Unbind();
}

void RendererInternal::RenderMesh(Mesh mesh, Material material) {
	mesh->Bind();
	int pass = material->GetPass();
	if (pass >= 0 && material->IsPassEnabled(pass)) {
		RenderMesh(mesh, material, pass);
	}
	else {
		for (int p = 0; p < material->GetPassCount(); ++p) {
			if (material->IsPassEnabled(p)) {
				RenderMesh(mesh, material, p);
			}
		}
	}

	mesh->Unbind();
}

void RendererInternal::RenderMesh(Mesh mesh, Material material, int pass) {
	material->Bind(pass);
	for (int i = 0; i < mesh->GetSubMeshCount(); ++i) {
		SubMesh subMesh = mesh->GetSubMesh(i);
		DrawCall(subMesh, mesh->GetTopology());
	}

	material->Unbind();
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

void RendererInternal::DrawCall(SubMesh subMesh, MeshTopology topology) {
	uint indexCount, baseVertex, baseIndex;
	subMesh->GetTriangles(indexCount, baseVertex, baseIndex);

	GLenum mode = TopologyToGLEnum(topology);
	GL::DrawElementsBaseVertex(mode, indexCount, GL_UNSIGNED_INT, (void*)(sizeof(uint)* baseIndex), baseVertex);
}

void RendererInternal::RenderSubMesh(Mesh mesh, int subMeshIndex, Material material, int pass) {
	material->Bind(pass);
	SubMesh subMesh = mesh->GetSubMesh(subMeshIndex);
	DrawCall(subMesh, mesh->GetTopology());

	material->Unbind();
}

void SkinnedMeshRendererInternal::UpdateMaterial(Entity entity) {
	for (int i = 0; i < GetMaterialCount(); ++i) {
		GetMaterial(i)->SetMatrix4Array(Variables::boneToRootSpaceMatrices, skeleton_->GetBoneToRootSpaceMatrices(), C_MAX_BONE_COUNT);
	}

	RendererInternal::UpdateMaterial(entity);
}

ParticleRendererInternal::ParticleRendererInternal()
	: RendererInternal(ObjectTypeParticleRenderer), particleCount_(0) {
	SetRenderQueue(RenderQueueTransparent);
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

void ParticleRendererInternal::DrawCall(SubMesh subMesh, MeshTopology topology) {
	if (particleCount_ == 0) { return; }
	uint indexCount, baseVertex, baseIndex;
	subMesh->GetTriangles(indexCount, baseVertex, baseIndex);

	GLenum mode = TopologyToGLEnum(topology);
	GL::DrawElementsInstancedBaseVertex(mode, indexCount, GL_UNSIGNED_INT, (void*)(sizeof(uint)* baseIndex), particleCount_, baseVertex);
}
