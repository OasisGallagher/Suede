#pragma once
#include "gizmos.h"

#include <vector>

#include "mesh.h"
#include "material.h"

struct GizmosBatch {
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
	virtual void DrawLines(const glm::vec3* points, uint npoints, uint* indexes, uint nindexes);
	virtual void DrawCuboid(const glm::vec3& center, const glm::vec3& size);

private:
	GizmosBatch& GetBatch();

private:
	Mesh mesh_;
	Material material_;

	glm::vec3 color_;
	std::vector<GizmosBatch> batches_;
};
