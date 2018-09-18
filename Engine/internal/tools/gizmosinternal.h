#pragma once
#include "gizmos.h"

#include <vector>

#include "mesh.h"
#include "material.h"

class GizmosInternal : public Gizmos {
public:
	GizmosInternal();

public:
	virtual void Flush();

	virtual Color GetColor() { return color_; }
	virtual void SetColor(const Color& value) { color_ = value; }
	
	virtual void DrawLines(const glm::vec3* points, uint npoints);
	virtual void DrawLines(const glm::vec3* points, uint npoints, const uint* indexes, uint nindexes);

	virtual void DrawLineStripe(const glm::vec3* points, uint npoints);
	virtual void DrawLineStripe(const glm::vec3* points, uint npoints, const uint* indexes, uint nindexes);

	virtual void DrawSphere(const glm::vec3& center, float radius);
	virtual void DrawCuboid(const glm::vec3& center, const glm::vec3& size);

	virtual void DrawWireSphere(const glm::vec3& center, float radius);
	virtual void DrawWireCuboid(const glm::vec3& center, const glm::vec3& size);

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

	void DrawGizmos(const Batch& b);

private:
	Mesh mesh_;

	Material lineMaterial_;

	Color color_;
	std::vector<Batch> batches_;
};
