#include <glm/glm.hpp>

class Plane {
public:
	Plane() :Plane(glm::vec4(0, 0, 1, 0)) {}

	Plane(const glm::vec3& normal, float d)
		: normal_(normal), d_(d) {
	}

	Plane(const glm::vec4& abcd) : Plane(glm::vec3(abcd), abcd.w) {
		Normalize();
	}

public:
	float GetDistance() const { return d_; }
	glm::vec3 GetNormal() const { return normal_; }

private:
	void Normalize() {
		float mag = glm::length(normal_);
		normal_ /= mag;
		d_ /= mag;
	}

	float d_;
	glm::vec3 normal_;
};
