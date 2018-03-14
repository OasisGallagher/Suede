#pragma once
#include "mesh.h"
#include "camera.h"
#include "texture.h"
#include "material.h"
#include "wrappers/gl.h"
#include "internal/base/framebuffer.h"

struct Renderable {
	uint instance;
	
	Mesh mesh;
	uint subMeshIndex;
	
	Material material;
	uint pass;

	FramebufferState state;

	glm::mat4 localToWorldMatrix;

	bool IsInstance(const Renderable& other) const {
		if (mesh->GetNativePointer() != other.mesh->GetNativePointer()) {
			return false;
		}

		if (subMeshIndex != other.subMeshIndex) {
			return false;
		}
		
		if (state.framebuffer != other.state.framebuffer) {
			return false;
		}

		if (material != other.material) {
			return false;
		}

		if (pass != other.pass) {
			return false;
		}

		return true;
	}
};

class Pipeline {
	enum RenderPass {
		RenderPassNone = -1,

		RenderPassShadowDepth,

		RenderPassForwardBackground,
		RenderPassForwardDepth,
		RenderPassForwardOpaque,
		RenderPassForwardTransparent,

		RenderPassDeferredGeometryPass,

		RenderPassCount
	};

public:
	Pipeline();

public:
	static Pipeline* GetCurrent() { return current_; }
	static void SetCurrent(Pipeline* value) { current_ = value; }

	static void SetCamera(Camera value) { camera_ = value; }
	static Camera GetCamera() { return camera_; }

	static void SetFramebuffer(FramebufferBase* value);
	static FramebufferBase* GetFramebuffer() { return framebuffer_; }

public:
	void Update();
	void AddRenderable(
		Mesh mesh,
		uint subMeshIndex,
		Material material,
		uint pass,
		const FramebufferState& state,
		const glm::mat4& localToWorldMatrix,
		uint instance = 0
	);

private:
	void ResetState();
	void SortRenderables();
	void Render(Renderable& ref);
	void RenderInstanced(uint first, uint last, const glm::mat4& worldToClipMatrix);
	void ClearRenderable(Renderable& ref);
	
private:
	uint nrenderables_;
	std::vector<Renderable> renderables_;

	int oldPass_;
	uint oldMeshPointer_;
	Material oldMaterial_;
	FramebufferState* oldTarget_;

	static Camera camera_;
	static Pipeline* current_;
	static FramebufferBase* framebuffer_;
};
