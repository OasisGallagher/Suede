#include "engine.h"
#include "physics.h"

#include "bullet/btBulletDynamicsCommon.h"
#include "internal/engine/subsysteminternal.h"

inline btVector3 btConvert(const Vector3& value) { return btVector3(value.x, value.y, value.z); }
inline btQuaternion btConvert(const Quaternion& value) { return btQuaternion(value.x, value.y, value.z, value.w); }

inline Vector3 btConvert(const btVector3& value) { return Vector3(value.x(), value.y(), value.z()); }
inline Quaternion btConvert(const btQuaternion& value) { return Quaternion(value.w(), value.x(), value.y(), value.z()); }

class PhysicsInternal : public SubsystemInternal {
public:
	PhysicsInternal(Gizmos* gizmos);
	~PhysicsInternal();

public:
	void FixedUpdate(float deltaTime);

	btDynamicsWorld* GetPhysicsWorld() { return world_; }
	bool Raycast(const Ray& ray, float maxDistance, RaycastHit* hitInfo);

	void SetGravity(const Vector3& value) { world_->setGravity(btConvert(value)); }
	Vector3 GetGravity() const { return btConvert(world_->getGravity()); }

	void SetDebugDrawEnabled(bool value) { debugDrawEnabled_ = value; }
	bool GetDebugDrawEnabled() const { return debugDrawEnabled_; }

private:
	bool debugDrawEnabled_ = false;

	btDynamicsWorld* world_;

	btBroadphaseInterface* broadphase_;
	btCollisionDispatcher* dispatcher_;
	btSequentialImpulseConstraintSolver* solver_;
	btDefaultCollisionConfiguration* collisionConfiguration_;
};
