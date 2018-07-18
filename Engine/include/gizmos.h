#pragma once
#include <glm/glm.hpp>

#include "types.h"
#include "tools/singleton.h"

class SUEDE_API Gizmos : public Singleton<Gizmos> {
public:
	glm::vec3 GetColor();
	void SetColor(const glm::vec3& value);

	void DrawLines(const glm::vec3* points, uint npoints);
	void DrawLines(const glm::vec3* points, uint npoints, uint* indexes, uint nindexes);

	void DrawCuboid(const glm::vec3& center, const glm::vec3& size);

public:
	void Flush();

public:
	Gizmos();

private:
	struct Batch {
		glm::vec3 color;
		std::vector<uint> indexes;
		std::vector<glm::vec3> points;
	};

private:
	Batch& GetBatch();

private:
	Mesh mesh_;
	Material material_;

	glm::vec3 color_;
	std::vector<Batch> batches_;
};
