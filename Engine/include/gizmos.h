#pragma once
#include <glm/glm.hpp>

#include "types.h"
#include "tools/singleton.h"

class SUEDE_API Gizmos : public Singleton2<Gizmos> {
public:
	virtual void Flush() = 0;

	virtual glm::vec3 GetColor() = 0;
	virtual void SetColor(const glm::vec3& value) = 0;

	virtual void DrawLines(const glm::vec3* points, uint npoints) = 0;
	virtual void DrawLines(const glm::vec3* points, uint npoints, uint* indexes, uint nindexes) = 0;

	virtual void DrawCuboid(const glm::vec3& center, const glm::vec3& size) = 0;
};
