#pragma once
#include "mesh.h"
#include "texture.h"
#include "material.h"
#include "wrappers/gl.h"
#include "internal/base/framebuffer.h"

struct Renderable {
	uint pass;
	uint instance;

	Mesh mesh;
	uint subMeshIndex;

	Material material;

	FramebufferState state;

	bool IsInstance(const Renderable& other) const {
		if (state.framebuffer != other.state.framebuffer) {
			return false;
		}

		if (material != other.material) {
			return false;
		}

		if (pass != other.pass) {
			return false;
		}

		if (mesh->GetNativePointer() != other.mesh->GetNativePointer()) {
			return false;
		}

		if (subMeshIndex != other.subMeshIndex) {
			return false;
		}

		return true;
	}
};

class Pipeline {
public:
	Pipeline();

public:
	static Pipeline* GetCurrent() { return current_; }
	static void SetCurrent(Pipeline* value) { current_ = value; }

public:
	void Update();
	Renderable* CreateRenderable();

private:
	void ResetState();
	void SortRenderables();
	void Render(Renderable& p);
	void RenderInstanced(uint first, uint last);
	void ClearRenderable(Renderable* renderable);
	
private:
	uint nrenderables_;
	std::vector<Renderable> renderables_;

	int oldPass_;
	uint oldMeshPointer_;
	Material oldMaterial_;
	FramebufferState* oldTarget_;

	static Pipeline* current_;
};
