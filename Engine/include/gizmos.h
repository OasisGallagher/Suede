#pragma once
#include <glm/glm.hpp>

#include "types.h"
#include "color.h"
#include "enginedefines.h"
#include "tools/singleton.h"

class SUEDE_API Gizmos : public Singleton2<Gizmos> {
	friend class Singleton<Gizmos>;
	SUEDE_DECLARE_IMPLEMENTATION(Gizmos)

public:
	void Flush();

	Color GetColor();
	void SetColor(const Color& value);

	void DrawLines(const glm::vec3* points, uint npoints);
	void DrawLines(const glm::vec3* points, uint npoints, const uint* indexes, uint nindexes);

	void DrawLineStripe(const glm::vec3* points, uint npoints);
	void DrawLineStripe(const glm::vec3* points, uint npoints, const uint* indexes, uint nindexes);

	void DrawSphere(const glm::vec3& center, float radius);
	void DrawCuboid(const glm::vec3& center, const glm::vec3& size);

	void DrawWireSphere(const glm::vec3& center, float radius);
	void DrawWireCuboid(const glm::vec3& center, const glm::vec3& size);

private:
	Gizmos();
};
