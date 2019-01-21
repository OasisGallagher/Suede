#pragma once
#include "gizmos.h"

#include <vector>

#include "mesh.h"
#include "material.h"
#include "frameeventlistener.h"

class GizmosInternal : public FrameEventListener {
public:	GizmosInternal();

public:
	void Flush();

	glm::mat4 GetMatrix() { return matrix_; }
	void SetMatrix(const glm::mat4& value) { matrix_ = value; }

	Color GetColor() { return color_; }
	void SetColor(const Color& value) { color_ = value; }
	
	void DrawLines(const glm::vec3* points, uint npoints);
	void DrawLines(const glm::vec3* points, uint npoints, const uint* indexes, uint nindexes);

	void DrawLineStripe(const glm::vec3* points, uint npoints);
	void DrawLineStripe(const glm::vec3* points, uint npoints, const uint* indexes, uint nindexes);

	void DrawSphere(const glm::vec3& center, float radius);
	void DrawCuboid(const glm::vec3& center, const glm::vec3& size);
	void DrawCircle(const glm::vec3& center, float radius, const glm::vec3& normal);

	void DrawWireSphere(const glm::vec3& center, float radius);
	void DrawWireCuboid(const glm::vec3& center, const glm::vec3& size);

public:
	void OnFrameLeave() { Flush(); }
	int GetFrameEventQueue() { return FrameEventQueueGizmos; }

private:
	struct Batch {
		MeshTopology topology;

		bool wireframe;
		Color color;
		Material material;

		std::vector<uint> indexes;
		std::vector<glm::vec3> points;
	};

private:
	Batch& GetBatch(MeshTopology topology, bool wireframe, Material material);
	bool IsBatchable(const Batch& ref, MeshTopology topology, bool wireframe, Material material);

	void FillBatch(Batch& b, const glm::vec3* points, uint npoints);
	void FillBatch(Batch& b, const glm::vec3* points, uint npoints, const uint* indexes, uint nindexes);

	void AddSphereBatch(const glm::vec3& center, float radius, bool wireframe);
	void AddCuboidBatch(const glm::vec3& center, const glm::vec3& size, bool wireframe);

	void AddArrowBatch(const glm::vec3& from, const glm::vec3& to);

	void DrawGizmos(const Batch& b);

private:
	Mesh mesh_;

	Material lineMaterial_;

	Color color_;
	glm::mat4 matrix_;
	std::vector<Batch> batches_;
};
