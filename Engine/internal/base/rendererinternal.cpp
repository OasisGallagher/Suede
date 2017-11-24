#include "variables.h"
#include "tools/math2.h"
#include "rendererinternal.h"
#include "internal/misc/timefinternal.h"

RendererInternal::RendererInternal(ObjectType type) : ObjectInternal(type), queue_(RenderQueueGeometry) {	
}

RendererInternal::~RendererInternal() {
}

void RendererInternal::RenderSprite(Sprite sprite) {
	UpdateMaterial(sprite);

	Mesh mesh = sprite->GetMesh();
	if (GetMaterialCount() == 1) {
		RenderMesh(mesh, GetMaterial(0));
	}
	else {
		RenderMesh(sprite->GetMesh());
	}
}

GLenum RendererInternal::TopologyToGLEnum(MeshTopology topology) {
	if (topology == MeshTopologyTriangles) { return GL_TRIANGLES; }
	return GL_TRIANGLE_STRIP;
}

void RendererInternal::UpdateMaterial(Sprite sprite) {
	int materialCount = GetMaterialCount();
	for (int i = 0; i < materialCount; ++i) {
		Material material = GetMaterial(i);
		material->SetFloat(Variables::time, timeInstance->GetRealTimeSinceStartup());
		material->SetFloat(Variables::deltaTime, timeInstance->GetDeltaTime());

		glm::mat4 localToWorldMatrix = sprite->GetLocalToWorldMatrix();
		material->SetMatrix4(Variables::localToWorldSpaceMatrix, localToWorldMatrix);
	}
}

void RendererInternal::RenderMesh(Mesh mesh) {
	int subMeshCount = mesh->GetSubMeshCount();
	int materialCount = GetMaterialCount();

	Assert(materialCount == subMeshCount);
	if (materialCount != subMeshCount) {
		Debug::LogError("material count mismatch with sub mesh count");
		return;
	}

	mesh->Bind();

	for (int i = 0; i < subMeshCount; ++i) {
		GetMaterial(i)->Bind();
		SubMesh subMesh = mesh->GetSubMesh(i);
		DrawCall(subMesh, mesh->GetTopology());

		GetMaterial(i)->Unbind();
	}

	mesh->Unbind();
}

void RendererInternal::RenderMesh(Mesh mesh, Material material) {
	mesh->Bind();
	material->Bind();

	for (int i = 0; i < mesh->GetSubMeshCount(); ++i) {
		SubMesh subMesh = mesh->GetSubMesh(i);
		DrawCall(subMesh, mesh->GetTopology());
	}

	mesh->Unbind();
	material->Unbind();
}

void RendererInternal::DrawCall(SubMesh subMesh, MeshTopology topology) {
	uint indexCount, baseVertex, baseIndex;
	subMesh->GetTriangles(indexCount, baseVertex, baseIndex);

	GLenum mode = TopologyToGLEnum(topology);
	glDrawElementsBaseVertex(mode, indexCount, GL_UNSIGNED_INT, (void*)(sizeof(uint)* baseIndex), baseVertex);
}

void SkinnedMeshRendererInternal::UpdateMaterial(Sprite sprite) {
	for (int i = 0; i < GetMaterialCount(); ++i) {
		GetMaterial(i)->SetMatrix4(Variables::boneToRootSpaceMatrices, *skeleton_->GetBoneToRootSpaceMatrices());
	}

	RendererInternal::UpdateMaterial(sprite);
}

ParticleRendererInternal::ParticleRendererInternal()
	: RendererInternal(ObjectTypeParticleRenderer), particleCount_(0) {
	SetRenderQueue(RenderQueueTransparent);
}

void ParticleRendererInternal::AddMaterial(Material material) {
	material->SetRenderState(Blend, SrcAlpha, OneMinusSrcAlpha);
	RendererInternal::AddMaterial(material);
}

void ParticleRendererInternal::RenderSprite(Sprite sprite) {
	ParticleSystem particleSystem = dsp_cast<ParticleSystem>(sprite);
	AssertX(particleSystem, "invalid particle system");
	particleCount_ = particleSystem->GetParticlesCount();

	RendererInternal::RenderSprite(sprite);
}

void ParticleRendererInternal::DrawCall(SubMesh subMesh, MeshTopology topology) {
	if (particleCount_ == 0) { return; }
	uint indexCount, baseVertex, baseIndex;
	subMesh->GetTriangles(indexCount, baseVertex, baseIndex);

	GLenum mode = TopologyToGLEnum(topology);
	glDrawElementsInstancedBaseVertex(mode, indexCount, GL_UNSIGNED_INT, (void*)(sizeof(uint)* baseIndex), particleCount_, baseVertex);
}
