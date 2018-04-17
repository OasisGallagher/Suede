#pragma once
#include "mesh.h"
#include "camera.h"
#include "texture.h"
#include "material.h"
#include "frameevent.h"
#include "wrappers/gl.h"
#include "internal/base/framebuffer.h"

struct Renderable {
	uint instance;

	Mesh mesh;
	uint subMeshIndex;
	
	Material material;
	uint pass;

	FramebufferState framebufferState;

	glm::mat4 localToWorldMatrix;

	void Clear();
	bool IsInstance(const Renderable& other) const;

	bool IsMeshInstanced(const Renderable& other) const;
	bool IsMaterialInstanced(const Renderable& other) const;
	bool IsFramebufferInstanced(const Renderable& other) const;
};

class Sample;
class Pipeline : public FrameEventListener {
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
	~Pipeline();

public:
	static Pipeline* GetCurrent() { return current_; }
	static void SetCurrent(Pipeline* value) { current_ = value; }

	static void SetCamera(Camera value) { camera_ = value; }
	static Camera GetCamera() { return camera_; }

	static void SetFramebuffer(FramebufferBase* value);
	static FramebufferBase* GetFramebuffer() { return framebuffer_; }

public:
	virtual void OnFrameEnter();
	virtual void OnFrameLeave();

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
	void Clear();

	void SortRenderables();
	void Render(Renderable& renderable);

	void ResetRenderContext();
	void UpdateRenderContext(Renderable& renderable);

	void GatherInstances(std::vector<uint>& ranges);
	void RenderInstances(uint first, uint last, const glm::mat4& worldToClipMatrix);

private:
	uint nrenderables_;
	std::vector<Renderable> renderables_;

	// render context.
	int oldPass_;
	Mesh oldMesh_;
	Material oldMaterial_;

	FramebufferState* oldFramebufferState_;

	// performance.
	uint ndrawcalls;
	Sample *switch_material, *switch_framebuffer, *switch_mesh, *update_ubo, *gather_instances, *update_pipeline, *sort_renderables, *rendering;

	// environment.
	static Camera camera_;
	static Pipeline* current_;
	static FramebufferBase* framebuffer_;
};
