#pragma once
#include <vector>
#include <glm/glm.hpp>

#include "types.h"

class SUEDE_API Geometries {
public:
	static void Circle(
		std::vector<glm::vec3>& points,
		const glm::vec3& center,
		float radius,
		const glm::vec3& normal,
		uint resolution
	);

	static void Sphere(
		std::vector<glm::vec3>& points, 
		std::vector<uint>& indexes, 
		const glm::ivec2& resolution
	);

	static void Ring(
		std::vector<glm::vec3>& points, 
		std::vector<uint>& indexes, 
		const glm::vec3& center, 
		float innerRadius, 
		float outterRadius,
		const glm::vec3& normal, 
		uint resolution
	);

	static void Cone(
		std::vector<glm::vec3>& points, 
		std::vector<uint>& indexes, 
		const glm::vec3& from, 
		const glm::vec3& to,
		float radius, 
		uint resolution
	);

	static void Cylinder(
		std::vector<glm::vec3>& points, 
		std::vector<uint>& indexes, 
		const glm::vec3& from,
		const glm::vec3& to,
		float radius, 
		uint resolution
	);

	static void Cuboid(
		std::vector<glm::vec3>& points,
		std::vector<uint>& indexes,
		const glm::vec3& center, 
		const glm::vec3& size
	);
};
