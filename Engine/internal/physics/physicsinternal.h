#include "engine.h"
#include "physics.h"

#include "mathconvert.h"
#include "bullet/btBulletDynamicsCommon.h"
#include "internal/engine/subsysteminternal.h"

class BulletDebugDrawer : public btIDebugDraw {
public:
	BulletDebugDrawer(Gizmos* gizmos) : selectionGizmos_(gizmos) {}

public:
	virtual void drawLine(const btVector3& from, const btVector3& to, const btVector3& color);
	virtual void drawContactPoint(const btVector3& PointOnB, const btVector3& normalOnB, btScalar distance, int lifeTime, const btVector3& color);

	virtual void reportErrorWarning(const char* warningString);

	virtual void setDebugMode(int debugMode) {}
	virtual void draw3dText(const btVector3& location, const char* textString) {}
	virtual int getDebugMode() const { return DBG_DrawWireframe | DBG_DrawAabb; }

private:
	Gizmos* selectionGizmos_;
};

class PhysicsInternal : public SubsystemInternal {
public:
	PhysicsInternal(Gizmos* gizmos);
	~PhysicsInternal();

public:
	static btDiscreteDynamicsWorld* btWorld() { return world_; }

public:
	void FixedUpdate(float deltaTime);

	bool Raycast(const Ray& ray, float maxDistance, RaycastHit* hitInfo);

	void SetGravity(const Vector3& value) { world_->setGravity(btConvert(value)); }
	Vector3 GetGravity() const { return btConvert(world_->getGravity()); }

	void SetDebugDrawEnabled(bool value) { debugDrawEnabled_ = value; }
	bool GetDebugDrawEnabled() const { return debugDrawEnabled_; }

private:
	void OnGameObjectComponentChanged(ref_ptr<GameObject> go, ComponentEventType state, ref_ptr<Component> component);

private:
	Gizmos* selectionGizmos_;

	bool debugDrawEnabled_;

	btBroadphaseInterface* broadphase_;
	btCollisionDispatcher* dispatcher_;
	btSequentialImpulseConstraintSolver* solver_;
	btDefaultCollisionConfiguration* collisionConfiguration_;

	static btDiscreteDynamicsWorld* world_;
};
