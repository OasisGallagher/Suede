#pragma once
#include "gizmos.h"

#include <vector>

#include "mesh.h"
#include "material.h"

struct GizmosBatch {
	MeshTopology topology;

	glm::vec3 color;
	std::vector<uint> indexes;
	std::vector<glm::vec3> points;
};

class GizmosInternal : public Gizmos {
public:
	GizmosInternal();

public:
	virtual void Flush();

	virtual glm::vec3 GetColor();
	virtual void SetColor(const glm::vec3& value);
	
	virtual void DrawLines(const glm::vec3* points, uint npoints);
	virtual void DrawLines(const glm::vec3* points, uint npoints, const uint* indexes, uint nindexes);

	virtual void DrawLineStripe(const glm::vec3* points, uint npoints);
	virtual void DrawLineStripe(const glm::vec3* points, uint npoints, const uint* indexes, uint nindexes);

	virtual void DrawSphere(const glm::vec3& center, float radius);


	virtual void DrawCuboid(const glm::vec3& center, const glm::vec3& size);

private:
	GizmosBatch& GetBatch(MeshTopology topology);

	void FillBatch(GizmosBatch &b, const glm::vec3* points, uint npoints);
	void FillBatch(GizmosBatch &b, const glm::vec3* points, uint npoints, uint nindexes, const uint* indexes);

	void RenderGizmos(const GizmosBatch& b);

	glm::vec3 SphereCoodrinate(float x, float y, float radius);
	void GetSphereCoodrinates(std::vector<glm::vec3>& points, float radius, const glm::vec3& center, const glm::ivec2& resolution);

private:
	Mesh mesh_;
	Material material_;

	glm::vec3 color_;
	std::vector<GizmosBatch> batches_;
};

inline glm::vec3 GizmosInternal::SphereCoodrinate(float x, float y, float radius) {
	return glm::vec3(cosf(x) * sinf(y), cosf(y), sinf(x) * sinf(y)) * radius;
}
