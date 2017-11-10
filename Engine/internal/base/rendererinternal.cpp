#include "variables.h"
#include "tools/math2.h"
#include "rendererinternal.h"
#include "internal/misc/timefinternal.h"

RendererInternal::RendererInternal(ObjectType type) : ObjectInternal(type), queue_(RenderQueueGeometry) {	
}

RendererInternal::~RendererInternal() {
}

void RendererInternal::RenderSprite(Sprite sprite) {
	for (int i = 0; i < GetMaterialCount(); ++i) {
		Material material = GetMaterial(i);
		material->SetFloat(Variables::time, timeInstance->GetRealTimeSinceStartup());
		material->SetFloat(Variables::deltaTime, timeInstance->GetDeltaTime());

		glm::mat4 localToWorldMatrix = sprite->GetLocalToWorldMatrix();
		material->SetMatrix4(Variables::localToWorldSpaceMatrix, localToWorldMatrix);
	}

	for (int i = 0; i < sprite->GetSurfaceCount(); ++i) {
		RenderSurface(sprite->GetSurface(i));
	}
}

GLenum RendererInternal::TopologyToGLEnum(MeshTopology topology) {
	if (topology == MeshTopologyTriangles) { return GL_TRIANGLES; }
	return GL_TRIANGLE_STRIP;
}

void RendererInternal::RenderSurface(Surface surface) {
	surface->Bind();

	// TODO: mesh count mismatch with material count.
	if (surface->GetMeshCount() > GetMaterialCount()) {
		Debug::LogWarning("mesh count mismatch with material count.");
	}

	// TODO: batch mesh if their materials are identical.
	for (int i = 0; i < GetMaterialCount(); ++i) {
		Mesh mesh = surface->GetMesh(Math::Min(i, surface->GetMeshCount() - 1));
		Material material = GetMaterial(i);

		material->Bind();
		DrawCall(mesh);
		material->Unbind();
	}

	surface->Unbind();
}

void RendererInternal::DrawCall(Mesh mesh) {
	unsigned vertexCount, baseVertex, baseIndex;
	mesh->GetTriangles(vertexCount, baseVertex, baseIndex);

	GLenum mode = TopologyToGLEnum(mesh->GetTopology());
	glDrawElementsBaseVertex(mode, vertexCount, GL_UNSIGNED_INT, (void*)(sizeof(unsigned)* baseIndex), baseVertex);
}

void SkinnedSurfaceRendererInternal::RenderSurface(Surface surface) {
	for (int i = 0; i < GetMaterialCount(); ++i) {
		GetMaterial(i)->SetMatrix4(Variables::boneToRootSpaceMatrices, *skeleton_->GetBoneToRootSpaceMatrices());
	}

	RendererInternal::RenderSurface(surface);
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

void ParticleRendererInternal::DrawCall(Mesh mesh) {
	if (particleCount_ == 0) { return; }
	unsigned vertexCount, baseVertex, baseIndex;
	mesh->GetTriangles(vertexCount, baseVertex, baseIndex);

	GLenum mode = TopologyToGLEnum(mesh->GetTopology());
	glDrawElementsInstancedBaseVertex(mode, vertexCount, GL_UNSIGNED_INT, (void*)(sizeof(unsigned)* baseIndex), particleCount_, baseVertex);
}
