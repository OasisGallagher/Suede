#include "variables.h"
#include "tools/math2.h"
#include "rendererinternal.h"
#include "internal/misc/timefinternal.h"

RendererInternal::RendererInternal(ObjectType type) : ObjectInternal(type), queue_(RenderQueueGeometry) {	
}

RendererInternal::~RendererInternal() {
}

void RendererInternal::RenderSprite(Sprite sprite) {
	int materialCount = GetMaterialCount();
	for (int i = 0; i < materialCount; ++i) {
		Material material = GetMaterial(i);
		material->SetFloat(Variables::time, timeInstance->GetRealTimeSinceStartup());
		material->SetFloat(Variables::deltaTime, timeInstance->GetDeltaTime());

		glm::mat4 localToWorldMatrix = sprite->GetLocalToWorldMatrix();
		material->SetMatrix4(Variables::localToWorldSpaceMatrix, localToWorldMatrix);
	}

	Assert(materialCount == 1 || materialCount == sprite->GetSurfaceCount());

	// TODO: relationship between material and surface...
	if (materialCount == 1) { GetMaterial(0)->Bind(); }

	for (int i = 0; i < sprite->GetSurfaceCount(); ++i) {
		if (materialCount > 1) { GetMaterial(i)->Bind(); }
		
		RenderSurface(sprite->GetSurface(i));

		if (materialCount > 1) { GetMaterial(i)->Unbind(); }
	}

	if (materialCount == 1) { GetMaterial(0)->Unbind(); }
}

GLenum RendererInternal::TopologyToGLEnum(MeshTopology topology) {
	if (topology == MeshTopologyTriangles) { return GL_TRIANGLES; }
	return GL_TRIANGLE_STRIP;
}

void RendererInternal::RenderSurface(Surface surface) {
	surface->Bind();

	for (int i = 0; i < surface->GetMeshCount(); ++i) {
		Mesh mesh = surface->GetMesh(i);
		DrawCall(mesh);
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
