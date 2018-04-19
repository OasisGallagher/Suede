#pragma once
#include "types.h"
#include <glm/glm.hpp>

class SUEDE_API Gizmos {
public:
	// TODO: UNUSED.
	static glm::vec3 GetColor();
	static void SetColor(const glm::vec3& value);

	static void DrawLines(const glm::vec3* points, uint npoints);
	static void DrawLines(const glm::vec3* points, uint npoints, uint* indexes, uint nindexes);

	static void DrawCuboid(const glm::vec3& center, const glm::vec3& size);

public:
	static void Flush();

private:
	Gizmos();
};
