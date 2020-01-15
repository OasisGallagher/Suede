#pragma once

#include "mesh.h"
#include "rect.h"
#include "camera.h"
#include "texture.h"
#include "material.h"
#include "tools/noncopyable.h"
#include "internal/base/framebuffer.h"

struct Renderable {
	uint instance;

	ref_ptr<Mesh> mesh;
	uint subMeshIndex;
	
	ref_ptr<Material> material;

	Matrix4 localToWorldMatrix;

	void Clear();
	bool IsInstance(const Renderable& other) const;

	bool IsMeshInstanced(const Renderable& other) const;
	bool IsMaterialInstanced(const Renderable& other) const;
};

enum class SortMode {
	ByMesh,
	BysMaterial,
	ByMaterialAndMesh,
};

class Sample;
class Profiler;
class RenderingContext;
class SharedUniformBuffers;

class Pipeline : private NonCopyable {
public:
	Pipeline(RenderingContext* context);
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
		const Matrix4& localToWorldMatrix,
		uint instance = 0
	);

	void AddRenderable(
		Mesh* mesh,
		uint subMeshIndex,
		Material* material,
		const Matrix4& localToWorldMatrix,
		uint instance = 0
	);

private:
	struct RenderingSamples {
		Sample* switchState;
		Sample* updateOffset;
		Sample* drawCall;
	};

	void RenderRange(uint from, uint to, int pass, RenderingSamples& renderSamples);
	void RenderInstanced(Renderable& renderable, uint instance, uint matrixOffset, int pass, RenderingSamples& samples);

	void ResetState();
	void UpdateState(Renderable& renderable, int pass);

	void UpdateMatrixBuffer(uint size, const void* data);

	void GatherInstances();
	void debugDumpPipelineAndRanges();

private:
	RenderingContext* context_;

	uint nrenderables_;
	std::vector<Renderable> renderables_;

	std::vector<uint> ranges_;
	std::vector<Matrix4> matrices_;

	Rect normalizedRect_;
	ref_ptr<RenderTexture> targetTexture_;

	Profiler* profiler_;
	TextureBuffer* matrixBuffer_;

	struct States {
		ref_ptr<Mesh> mesh;

		int pass = -1;
		ref_ptr<Material> material;

		void Reset();
	} oldStates_;

	struct {
		uint drawcalls;
		uint triangles;

		uint meshChanges;
		uint materialChanges;
	} counters_;
};
