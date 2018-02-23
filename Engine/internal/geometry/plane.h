#include <glm/glm.hpp>

class Plane {
public:
	Plane() :Plane(glm::vec4(0, 0, 1, 0)) {}
	Plane(const glm::vec4& abcd);
	Plane(const glm::vec3& normal, float d) : normal_(normal), d_(d) {}

public:
	float GetDistance() const { return d_; }
	glm::vec3 GetNormal() const { return normal_; }

private:
	void Normalize();

private:
	float d_;
	glm::vec3 normal_;
};
