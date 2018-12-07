#include "tools/singleton.h"

class SUEDE_API Physics : private Singleton2<Physics> {
	friend class Singleton<Physics>;
	SUEDE_DECLARE_IMPLEMENTATION(Physics)

public:
	static void SetGravity(const glm::vec3& value);
	static glm::vec3 GetGravity();

	static void SetDebugDrawEnabled(bool value);
	static bool GetDebugDrawEnabled();

private:
	Physics();
};
