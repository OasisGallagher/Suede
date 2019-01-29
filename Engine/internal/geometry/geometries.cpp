#include "geometries.h"

#include "tools/math2.h"

#define ADD_TRIANGLE(c, i, j, k)	\
	c.push_back(i);	c.push_back(j); c.push_back(k)

#define SPHERE_COODRINATE(x, y) \
	glm::vec3(Math::Cos(x) * Math::Sin(y), Math::Cos(y), Math::Sin(x) * Math::Sin(y))

static void CircleSegment(std::vector<glm::vec3>& points, std::vector<uint>& indexes, const glm::vec3& from, const glm::vec3& to, const glm::vec3& normal, const glm::vec2& size) {
	glm::vec3 up = to - from;
	float height = glm::length(up);
	up /= 2;

	glm::vec3 right = normal;
	right *= size.x / 2;

	glm::vec3 forward = glm::normalize(glm::cross(right, up));
	forward *= size.y / 2;

	uint first = points.size();
	glm::vec3 center = (from + to) / 2.f;

	points.insert(points.end(), {
		center + up + right - forward,
		center + up - right - forward,
		center + up - right + forward,
		center + up + right + forward,
		center - up + right - forward,
		center - up - right - forward,
		center - up - right + forward,
		center - up + right + forward,
	});

	indexes.insert(indexes.end(), {
		//0 + first, 1 + first, 2 + first,
		//0 + first, 2 + first, 3 + first,
		0 + first, 4 + first, 5 + first,
		0 + first, 5 + first, 1 + first,
		1 + first, 5 + first, 6 + first,
		1 + first, 6 + first, 2 + first,
		2 + first, 6 + first, 7 + first,
		2 + first, 7 + first, 3 + first,
		3 + first, 7 + first, 4 + first,
		3 + first, 4 + first, 0 + first,
		//4 + first, 7 + first, 6 + first,
		//4 + first, 6 + first, 5 + first
	});
}

void Geometries::Circle(std::vector<glm::vec3>& points, const glm::vec3& center, float radius, const glm::vec3& normal, uint resolution) {
	float step = Math::Pi2 / resolution;
	glm::vec3 forward(0, 1, 0);
	if (!Math::Approximately(normal.y, 0) || !Math::Approximately(normal.z, 0)) {
		forward = glm::vec3(1, 0, 0);
	}

	forward = glm::normalize(glm::cross(forward, normal));

	for (int i = 0; i < resolution; ++i) {
		glm::quat q = glm::angleAxis(i * step, normal);
		points.push_back(q * forward * radius + center);
	}
}

void Geometries::Rectangle(std::vector<glm::vec3>& points, std::vector<uint>& indexes, const glm::vec3& from, const glm::vec3& to, const glm::vec3& normal, float width) {
	glm::vec3 forward = to - from;
	float height = glm::length(forward);
	forward /= 2;

	glm::vec3 right = glm::normalize(glm::cross(forward, normal)) * width / 2.f;

	uint first = points.size();

	glm::vec3 center = (from + to) / 2.f;
	points.push_back(center + forward - right);
	points.push_back(center + forward + right);
	points.push_back(center - forward - right);
	points.push_back(center - forward + right);

	ADD_TRIANGLE(indexes, first, first + 2, first + 3);
	ADD_TRIANGLE(indexes, first + 1, first, first + 3);
}

void Geometries::Sphere(std::vector<glm::vec3>& points, std::vector<uint>& indexes, const glm::ivec2& resolution) {
	// step size between U-points on the grid
	glm::vec2 step = glm::vec2(Math::Pi2, Math::Pi) / glm::vec2(resolution);

	for (float i = 0; i < resolution.x; ++i) { // U-points
		for (float j = 0; j < resolution.y; ++j) { // V-points
			glm::vec2 uv = glm::vec2(i, j) * step;
			float un = ((i + 1) == resolution.x) ? Math::Pi2 : (i + 1) * step.x;
			float vn = ((j + 1) == resolution.y) ? Math::Pi : (j + 1) * step.y;

			// Find the four points of the grid square by evaluating the parametric urface function.
			glm::vec3 p[] = {
				SPHERE_COODRINATE(uv.x, uv.y),
				SPHERE_COODRINATE(uv.x, vn),
				SPHERE_COODRINATE(un, uv.y),
				SPHERE_COODRINATE(un, vn)
			};

			uint c = points.size();
			ADD_TRIANGLE(indexes, c + 0, c + 2, c + 1);
			ADD_TRIANGLE(indexes, c + 3, c + 1, c + 2);

			points.insert(points.end(), p, p + SUEDE_COUNTOF(p));
		}
	}
}

void Geometries::Ring(std::vector<glm::vec3>& points, std::vector<uint>& indexes, const glm::vec3& center, float innerRadius, float outterRadius, const glm::vec3& normal, uint resolution) {
	float step = Math::Pi2 / resolution;
	glm::vec3 forward(0, 1, 0);
	if (!Math::Approximately(normal.y, 0) || !Math::Approximately(normal.z, 0)) {
		forward = glm::vec3(1, 0, 0);
	}

	forward = glm::normalize(glm::cross(forward, normal));

	float cylinderRadius = (outterRadius - innerRadius) / 2;
	float radius = innerRadius + cylinderRadius;
	glm::vec3 from = forward * radius + center;
	for (int i = 1; i < resolution + 1; ++i) {
		glm::quat q = glm::angleAxis(i * step, normal);
		glm::vec3 current = q * forward * radius + center;
		Cylinder(points, indexes, from, current, cylinderRadius, resolution);
		from = current;
	}
}

void Geometries::Circle(std::vector<glm::vec3>& points, std::vector<uint>& indexes, const glm::vec3& center, float radius, float width, const glm::vec3& normal, uint resolution) {
	float step = Math::Pi2 / resolution;
	glm::vec3 forward(0, 1, 0);
	if (!Math::Approximately(normal.y, 0) || !Math::Approximately(normal.z, 0)) {
		forward = glm::vec3(1, 0, 0);
	}

	forward = glm::normalize(glm::cross(forward, normal));

	glm::vec3 from = forward * radius + center;
	for (int i = 1; i < resolution + 1; ++i) {
		glm::quat q = glm::angleAxis(i * step, normal);
		glm::vec3 current = q * forward * radius + center;
		CircleSegment(points, indexes, from, current, glm::normalize(glm::cross(normal, current - from)), glm::vec2(width));
		from = current;
	}
}

void Geometries::Cone(std::vector<glm::vec3>& points, std::vector<uint>& indexes, const glm::vec3& from, const glm::vec3& to, float radius, uint resolution) {
	glm::vec3 normal = glm::normalize(to - from);

	int first = points.size();
	Circle(points, from, radius, normal, resolution);

	int last = points.size();
	points.insert(points.end(), { from, to });

	for (int i = first + 1; i < last; ++i) {
		ADD_TRIANGLE(indexes, last + 1, i - 1, i);
		ADD_TRIANGLE(indexes, last, i, i - 1);
	}

	ADD_TRIANGLE(indexes, last, last - 1, first);
	ADD_TRIANGLE(indexes, last + 1, first, last - 1);
}

void Geometries::Cylinder(std::vector<glm::vec3>& points, std::vector<uint>& indexes, const glm::vec3& from, const glm::vec3& to, float radius, uint resolution) {
	glm::vec3 dir = to - from;

	int first = points.size();
	Circle(points, from, radius, glm::normalize(dir), resolution);

	int last = points.size();
	for (int i = first; i < last; ++i) {
		points.push_back(points[i] + dir);
	}

	int last2 = points.size();
	points.insert(points.end(), { from, to });

	for (int i = first + 1; i < last; ++i) {
		int step = i - first;
		ADD_TRIANGLE(indexes, i - 1, last + step - 1, i);
		ADD_TRIANGLE(indexes, i, last + step - 1, last + step);
		ADD_TRIANGLE(indexes, last2, i - 1, i);
		ADD_TRIANGLE(indexes, last2 + 1, last + step - 1, last + step);
	}

	ADD_TRIANGLE(indexes, last - 1, last2 - 1, last);
	ADD_TRIANGLE(indexes, first, last - 1, last);
	ADD_TRIANGLE(indexes, last2, last - 1, first);
	ADD_TRIANGLE(indexes, last2 + 1, last2 - 1, last);
}

void Geometries::Cuboid(std::vector<glm::vec3>& points, std::vector<uint>& indexes, const glm::vec3& center, const glm::vec3& size) {
	glm::vec3 half = size / 2.f;
	uint first = points.size();

	points.insert(points.end(), {
		center + glm::vec3(half.xy, -half.z),
		center + glm::vec3(-half.x, half.y, -half.z),
		center + glm::vec3(-half.x, half.yz),
		center + half,
		center + glm::vec3(half.x, glm::vec2(0) - half.yz),
		center + (-half),
		center + glm::vec3(glm::vec2(0) - half.xy, half.z),
		center + glm::vec3(half.x, -half.y, half.z),
	});

	indexes.insert(indexes.end(), {
		0 + first, 1 + first, 2 + first, 
		0 + first, 2 + first, 3 + first, 
		0 + first, 4 + first, 5 + first, 
		0 + first, 5 + first, 1 + first,
		1 + first, 5 + first, 6 + first, 
		1 + first, 6 + first, 2 + first, 
		2 + first, 6 + first, 7 + first,
		2 + first, 7 + first, 3 + first,
		3 + first, 7 + first, 4 + first,
		3 + first, 4 + first, 0 + first,
		4 + first, 7 + first, 6 + first,
		4 + first, 6 + first, 5 + first
	});
}
