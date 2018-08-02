#pragma once
#include "../api/gl.h"

#include "mesh.h"
#include "rect.h"
#include "camera.h"
#include "texture.h"
#include "material.h"
#include "frameeventlistener.h"

#include "internal/base/gpuquerier.h"
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
class Pipeline : public QuerierResultListener {
public:
	Pipeline();
	~Pipeline();

public:
	void Sort(SortMode mode, const glm::mat4& worldToClipMatrix);

	void Run(bool __tmpIsRendering = false);
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
	virtual void OnQuerierResult(uint id, uint result);

private:
	void Render(Renderable& renderable, uint instance, uint matrixOffset);

	void ResetState();
	void UpdateState(Renderable& renderable);

	void RenderInstances(uint first, uint last);
	void GatherInstances(std::vector<uint>& ranges);
	void debugDumpPipelineAndRanges(std::vector<uint>& ranges);

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

	uint nmeshChanges_;
	uint nmaterialChanges_;

	Rect normalizedRect_;
	RenderTexture targetTexture_;

	uint timeQuerier_;

	// performance.
	Sample 
		*switch_state,
		*update_ubo, 
		*update_offset, 
		*update_matrices, 
		*update_tbo, 
		*draw_call,
		*gather_instances, 
		*update_pipeline, 
		*stat_and_output;
};
