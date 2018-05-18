#pragma once
#include "api/gl.h"

#include "mesh.h"
#include "rect.h"
#include "camera.h"
#include "texture.h"
#include "material.h"
#include "frameeventlistener.h"
#include "internal/base/framebuffer.h"

struct Renderable {
	uint instance;

	Mesh mesh;
	uint subMeshIndex;
	
	Material material;
	uint pass;

	RenderTexture target;
	Rect normalizedRect;

	glm::mat4 localToWorldMatrix;

	bool operator < (const Renderable& other) const;

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
	virtual void OnFrameEnter();
	virtual void OnFrameLeave();

public:
	void Flush(const glm::mat4& worldToClipMatrix);
	
	void AddRenderable(
		Mesh mesh,
		Material material,
		uint pass,
		RenderTexture target,
		const Rect& normalizedRect,
		const glm::mat4& localToWorldMatrix,
		uint instance = 0
	);

	void AddRenderable(
		Mesh mesh,
		uint subMeshIndex,
		Material material,
		uint pass,
		RenderTexture target,
		const Rect& normalizedRect,
		const glm::mat4& localToWorldMatrix,
		uint instance = 0
	);

private:
	void Clear();
	void debugDumpPipelineAndRanges(std::vector<uint>& ranges);

	void SortRenderables();
	void Render(Renderable& renderable);

	void ResetState();
	void UpdateState(Renderable& renderable);

	void GatherInstances(std::vector<uint>& ranges);
	void RenderInstances(uint first, uint last, const glm::mat4& worldToClipMatrix);

private:
	uint nrenderables_;
	std::vector<Renderable> renderables_;

	// states.
	int oldPass_;
	Mesh oldMesh_;
	Material oldMaterial_;
	RenderTexture oldTarget_;

	uint ndrawcalls_;
	uint ntriangles_;

	// performance.
	Sample *switch_material, *switch_framebuffer, *switch_mesh, *update_ubo, *gather_instances, *update_pipeline, *sort_renderables, *rendering;
};
