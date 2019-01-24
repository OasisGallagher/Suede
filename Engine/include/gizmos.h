#pragma once
#include <glm/glm.hpp>

#include "types.h"
#include "color.h"
#include "enginedefines.h"
#include "tools/singleton.h"

class SUEDE_API Gizmos : private singleton2<Gizmos> {
	friend class singleton<Gizmos>;
	SUEDE_DECLARE_IMPLEMENTATION(Gizmos)

public:
	static void Flush();

	static glm::mat4 GetMatrix();
	static void SetMatrix(const glm::mat4& value);

	static Color GetColor();
	static void SetColor(const Color& value);

	static void DrawLines(const glm::vec3* points, uint npoints);
	static void DrawLines(const std::initializer_list<glm::vec3>& points);
	static void DrawLines(const glm::vec3* points, uint npoints, const uint* indexes, uint nindexes);
	static void DrawLines(const std::initializer_list<glm::vec3>& points, const std::initializer_list<uint>& indexes);

	static void DrawLineStripe(const glm::vec3* points, uint npoints);
	static void DrawLineStripe(const glm::vec3* points, uint npoints, const uint* indexes, uint nindexes);

	static void DrawCircle(const glm::vec3& center, float radius, const glm::vec3& normal);

	static void DrawSphere(const glm::vec3& center, float radius);
	static void DrawCuboid(const glm::vec3& center, const glm::vec3& size);

	static void DrawWireSphere(const glm::vec3& center, float radius);
	static void DrawWireCuboid(const glm::vec3& center, const glm::vec3& size);

private:
	Gizmos();
};
