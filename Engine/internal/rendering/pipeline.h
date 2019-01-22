#pragma once
#include "../api/gl.h"

#include <glm/glm.hpp>

#include "mesh.h"
#include "rect.h"
#include "camera.h"
#include "texture.h"
#include "material.h"
#include "matrixbuffer.h"

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
	Pipeline(MatrixBuffer* matrixBuffer);
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
	void Render(Renderable& renderable, uint instance, uint matrixOffset);

	void ResetState();
	void UpdateState(Renderable& renderable);

	void RenderInstances(uint first, uint last);
	void GatherInstances(std::vector<uint>& ranges);
	void debugDumpPipelineAndRanges(std::vector<uint>& ranges);

private:
	MatrixBuffer* matrixBuffer_;

	uint nrenderables_;
	std::vector<Renderable> renderables_;

	std::vector<uint> ranges_;
	std::vector<glm::mat4> matrices_;

	Rect normalizedRect_;
	RenderTexture targetTexture_;

	struct States {
		int pass;
		Mesh mesh;
		Material material;

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
