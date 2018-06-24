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

	glm::mat4 localToWorldMatrix;

	void Clear();
	bool IsInstance(const Renderable& other) const;

	bool IsMeshInstanced(const Renderable& other) const;
	bool IsMaterialInstanced(const Renderable& other) const;
};

enum SortMode {
	SortModeMesh,
	SortModeMaterial,
	SortModeMeshMaterial,
};

class Sample;
class Pipeline {
public:
	Pipeline();
	~Pipeline();

public:
	void Sort(SortMode mode, const glm::mat4& worldToClipMatrix);

	void Run();
	void Clear();

	RenderTexture GetTargetTexture();
	void SetTargetTexture(RenderTexture value, const Rect& normalizedRect);

	Pipeline& operator = (const Pipeline& other);

	uint GetRenderableCount() const { return nrenderables_; }
	Renderable& GetRenderable(uint i) { return renderables_[i]; }

	void AddRenderable(
		Mesh mesh,
		Material material,
		uint pass,
		const glm::mat4& localToWorldMatrix,
		uint instance = 0
	);

	void AddRenderable(
		Mesh mesh,
		uint subMeshIndex,
		Material material,
		uint pass,
		const glm::mat4& localToWorldMatrix,
		uint instance = 0
	);

private:
	void Render(Renderable& renderable, uint instance);

	void ResetState();
	void UpdateState(Renderable& renderable);

	void RenderInstances(uint first, uint last);
	void GatherInstances(std::vector<uint>& ranges);

private:
	uint nrenderables_;
	std::vector<Renderable> renderables_;

	std::vector<uint> ranges_;
	std::vector<glm::mat4> matrices_;

	// states.
	int oldPass_;
	Mesh oldMesh_;
	Material oldMaterial_;

	uint ndrawcalls_;
	uint ntriangles_;

	Rect normalizedRect_;
	RenderTexture targetTexture_;

	// performance.
	Sample *switch_material, *switch_mesh, *update_ubo, *update_matrices, *gather_instances, *update_pipeline, *rendering;
};
