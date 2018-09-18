#pragma once
#include <glm/glm.hpp>

#include "types.h"
#include "color.h"
#include "tools/singleton.h"

class SUEDE_API Gizmos : public Singleton2<Gizmos> {
public:
	virtual void Flush() = 0;

	virtual Color GetColor() = 0;
	virtual void SetColor(const Color& value) = 0;

	virtual void DrawLines(const glm::vec3* points, uint npoints) = 0;
	virtual void DrawLines(const glm::vec3* points, uint npoints, const uint* indexes, uint nindexes) = 0;

	virtual void DrawLineStripe(const glm::vec3* points, uint npoints) = 0;
	virtual void DrawLineStripe(const glm::vec3* points, uint npoints, const uint* indexes, uint nindexes) = 0;

	virtual void DrawSphere(const glm::vec3& center, float radius) = 0;
	virtual void DrawCuboid(const glm::vec3& center, const glm::vec3& size) = 0;

	virtual void DrawWireSphere(const glm::vec3& center, float radius) = 0;
	virtual void DrawWireCuboid(const glm::vec3& center, const glm::vec3& size) = 0;
};
