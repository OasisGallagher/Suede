#include "variables.h"
#include "renderstate.h"
#include "rendererinternal.h"
#include "internal/misc/timefinternal.h"

RendererInternal::RendererInternal(ObjectType type) : ObjectInternal(type), queue_(RenderQueueGeometry) {
	std::fill(states_, states_ + RenderStateCount, nullptr);
}

RendererInternal::~RendererInternal() {
	for (int i = 0; i < RenderStateCount; ++i) {
		Memory::Release(states_[i]);
	}
}

void RendererInternal::RenderSprite(Sprite sprite) {
	Material material = GetMaterial(0);

	material->SetFloat(Variables::time, timeInstance->GetRealTimeSinceStartup());
	material->SetFloat(Variables::deltaTime, timeInstance->GetDeltaTime());

	glm::mat4 localToWorldMatrix = sprite->GetLocalToWorldMatrix();
	material->SetMatrix4(Variables::localToWorldSpaceMatrix, localToWorldMatrix);

	RenderSurface(sprite->GetSurface());
}

void RendererInternal::RenderSurface(Surface surface) {
	BindRenderStates();
	DrawSurface(surface);
	UnbindRenderStates();
}

void RendererInternal::SetRenderState(RenderStateType type, int parameter0, int parameter1) {
	RenderState* state = nullptr;
	switch (type) {
		case Cull:
			state = Memory::Create<CullState>();
			break;
		case DepthTest:
			state = Memory::Create<DepthTestState>();
			break;
		case Blend:
			state = Memory::Create<BlendState>();
			break;
		case DepthWrite:
			state = Memory::Create<DepthWriteState>();
			break;
		case RasterizerDiscard:
			state = Memory::Create<RasterizerDiscardState>();
			break;
		default:
			Debug::LogError("invalid render capacity " + std::to_string(type));
			break;
	}

	state->Initialize(parameter0, parameter1);
	Memory::Release(states_[type]);
	states_[type] = state;
}


GLenum RendererInternal::PrimaryTypeToGLEnum(PrimaryType type) {
	if (type == PrimaryTypeTriangle) { return GL_TRIANGLES; }
	return GL_TRIANGLE_STRIP;
}

void RendererInternal::DrawSurface(Surface surface) {
	surface->Bind();

	// TODO: batch mesh if their materials are identical.
	for (int i = 0; i < surface->GetMeshCount(); ++i) {
		Mesh mesh = surface->GetMesh(i);

		for (int j = 0; j < GetMaterialCount(); ++j) {
			DrawMesh(mesh, GetMaterial(j));
		}
	}

	surface->Unbind();
}

void RendererInternal::DrawMesh(Mesh mesh, Material material) {
	MaterialTextures& textures = mesh->GetMaterialTextures();

	if (textures.bump) {
		material->SetTexture(Variables::bumpTexture, textures.bump);
	}

	if (textures.albedo) {
		material->SetTexture(Variables::mainTexture, textures.albedo);
	}

	if (textures.specular) {
		material->SetTexture(Variables::specularTexture, textures.specular);
	}

	material->Bind();
	DrawCall(mesh);
	material->Unbind();
}

void RendererInternal::DrawCall(Mesh mesh) {
	unsigned vertexCount, baseVertex, baseIndex;
	mesh->GetTriangles(vertexCount, baseVertex, baseIndex);

	GLenum mode = PrimaryTypeToGLEnum(mesh->GetPrimaryType());
	glDrawElementsBaseVertex(mode, vertexCount, GL_UNSIGNED_INT, (void*)(sizeof(unsigned)* baseIndex), baseVertex);
}

void RendererInternal::BindRenderStates() {
	for (int i = 0; i < RenderStateCount; ++i) {
		if (states_[i] != nullptr) {
			states_[i]->Bind();
		}
	}
}

void RendererInternal::UnbindRenderStates() {
	for (int i = 0; i < RenderStateCount; ++i) {
		if (states_[i] != nullptr) {
			states_[i]->Unbind();
		}
	}
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
	SetRenderState(Blend, SrcAlpha, OneMinusSrcAlpha);
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

	GLenum mode = PrimaryTypeToGLEnum(mesh->GetPrimaryType());
	// TODO: 
	
	glVertexAttribDivisor(0, 0); // particles vertices : always reuse the same 4 vertices -> 0
	glVertexAttribDivisor(6, 1); // positions : one per quad (its center)                 -> 1
	glVertexAttribDivisor(7, 1); // color : one per quad                                  -> 1
	
	glDrawElementsInstancedBaseVertex(mode, vertexCount, GL_UNSIGNED_INT, (void*)(sizeof(unsigned)* baseIndex), particleCount_, baseVertex);
}
