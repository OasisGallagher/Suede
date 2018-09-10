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

	virtual glm::vec3 GetColor() { return color_; }
	virtual void SetColor(const glm::vec3& value) { color_ = value; }
	
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
		glm::vec3 color;
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

	glm::vec3 SphereCoodrinate(float x, float y);
	void GetSphereCoodrinates(std::vector<glm::vec3>& points, std::vector<uint>& indexes, const glm::ivec2& resolution);

private:
	Mesh mesh_;

	Material lineMaterial_;

	glm::vec3 color_;
	std::vector<Batch> batches_;
};

inline glm::vec3 GizmosInternal::SphereCoodrinate(float x, float y) {
	return glm::vec3(cosf(x) * sinf(y), cosf(y), sinf(x) * sinf(y));
}
