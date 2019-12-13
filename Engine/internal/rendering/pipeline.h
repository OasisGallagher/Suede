#pragma once

#include "mesh.h"
#include "rect.h"
#include "camera.h"
#include "texture.h"
#include "material.h"

#include "internal/base/framebuffer.h"

struct Renderable {
	uint instance;

	ref_ptr<Mesh> mesh;
	uint subMeshIndex;
	
	ref_ptr<Material> material;
	uint pass;

	Matrix4 localToWorldMatrix;

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
class Context;
class SharedUniformBuffers;

class Pipeline : private NonCopyable {
public:
	Pipeline(Context* context);
	~Pipeline();

public:
	void Sort(SortMode mode, const Matrix4& worldToClipMatrix);

	void Run();
	void Clear();

	RenderTexture* GetTargetTexture();
	void AssignRenderables(const Pipeline* other);
	void SetTargetTexture(RenderTexture* value, const Rect& normalizedRect);

	uint GetRenderableCount() const { return nrenderables_; }
	Renderable& GetRenderable(uint i) { return renderables_[i]; }

	void AddRenderable(
		Mesh* mesh,
		Material* material,
		uint pass,
		const Matrix4& localToWorldMatrix,
		uint instance = 0
	);

	void AddRenderable(
		Mesh* mesh,
		uint subMeshIndex,
		Material* material,
		uint pass,
		const Matrix4& localToWorldMatrix,
		uint instance = 0
	);

private:
	void Render(Renderable& renderable, uint instance, uint matrixOffset);

	void ResetState();
	void UpdateState(Renderable& renderable);

	void UpdateMatrixBuffer(uint size, const void* data);

	void RenderInstances(uint first, uint last);
	void GatherInstances(std::vector<uint>& ranges);
	void debugDumpPipelineAndRanges(std::vector<uint>& ranges);

private:
	Context* context_;

	uint nrenderables_;
	std::vector<Renderable> renderables_;

	std::vector<uint> ranges_;
	std::vector<Matrix4> matrices_;

	Rect normalizedRect_;
	ref_ptr<RenderTexture> targetTexture_;

	TextureBuffer* matrixBuffer_;

	struct States {
		int pass;
		ref_ptr<Mesh> mesh;
		ref_ptr<Material> material;

		void Reset();
	} oldStates_;

	struct {
		uint drawcalls;
		uint triangles;

		uint meshChanges;
		uint materialChanges;
	} counters_;

	// profiler samples.
	struct Samples {
		Samples();
		~Samples();

		Sample* switch_state;
		Sample*	update_ubo;
		Sample* update_offset;
		Sample* update_matrices;
		Sample* update_tbo;
		Sample* draw_call;
		Sample* gather_instances;
		Sample* update_pipeline;
		Sample* stat_and_output;
		Sample* reset_states;

		void Reset();
	} samples_;
};
