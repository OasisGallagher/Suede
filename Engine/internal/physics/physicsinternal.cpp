#include "physicsinternal.h"

#include "time2.h"
#include "gizmos.h"
#include "rigidbody.h"
#include "memory/refptr.h"
#include "rigidbodyinternal.h"
#include "internal/gameobject/gameobjectinternal.h"

class BulletDebugDrawer : public btIDebugDraw {
public:
	BulletDebugDrawer(Gizmos* gizmos) : gizmos_(gizmos) {}

public:
	virtual void drawLine(const btVector3& from, const btVector3& to, const btVector3& color) {
		Color oldColor = gizmos_->GetColor();
		gizmos_->SetColor(Color(color.x(), color.y(), color.z()));
		gizmos_->DrawLines({ btConvert(from), btConvert(to) });
		gizmos_->SetColor(oldColor);
	}

	virtual void reportErrorWarning(const char* warningString) {
		Debug::LogError(warningString);
	}

	virtual void setDebugMode(int debugMode) {}
	virtual void draw3dText(const btVector3& location, const char* textString) {}
	virtual int getDebugMode() const { return DBG_DrawAabb; }
	virtual void drawContactPoint(const btVector3& PointOnB, const btVector3& normalOnB, btScalar distance, int lifeTime, const btVector3& color) {}

private:
	Gizmos* gizmos_;
};

Physics::Physics(Gizmos* gizmos) : Subsystem(new PhysicsInternal(gizmos)) {}
bool Physics::Raycast(const Ray& ray, float maxDistance, RaycastHit* hitInfo) { return _suede_dptr()->Raycast(ray, maxDistance, hitInfo); }
void Physics::SetGravity(const Vector3& value) { _suede_dptr()->SetGravity(value); }
void Physics::Update(float deltaTime) { _suede_dptr()->FixedUpdate(deltaTime); }
Vector3 Physics::GetGravity() { return _suede_dptr()->GetGravity(); }
void Physics::SetDebugDrawEnabled(bool value) { _suede_dptr()->SetDebugDrawEnabled(value); }
bool Physics::GetDebugDrawEnabled() { return _suede_dptr()->GetDebugDrawEnabled(); }

PhysicsInternal::PhysicsInternal(Gizmos* gizmos) {
	// You instantiate the broad phase algorithm implementation.
	// Collision detection is done in two phases : broad and narrow.
	// In the broad phase, the physics engine quickly eliminates objects that cannot collide.
	// For example, it can run a quick check to using the bounding boxes of objects,
	// eliminating those that don¡¯t collide.It then passes only a small list of objects that 
	// can collide to the narrow phase, which is much slower, since it checks actual shapes for collision.
	// Bullet has several built - in implementations of the broad phase.In this tutorial, 
	// you¡¯re using the dynamic AABB tree implementation ¨C i.e.btDbvtBroadphase.
	broadphase_ = new btDbvtBroadphase;

	// collisionConfiguration is responsible for full, not broad, collision detection.
	// In other words, this is where the more fine - grained and accurate collision detection code runs.
	// You could create your own implementation, but for now you¡¯re using the built - in configuration again.
	collisionConfiguration_ = new btDefaultCollisionConfiguration;
	dispatcher_ = new btCollisionDispatcher(collisionConfiguration_);

	// This is what causes the objects to interact properly, taking into account gravity, game logic supplied forces,
	// collisions, and hinge constraints. It does a good job as long as you don¡¯t push it to extremes, and is one of 
	// the bottlenecks in any high performance simulation. There are parallel versions available for some threading models.
	solver_ = new btSequentialImpulseConstraintSolver;

	world_ = new btDiscreteDynamicsWorld(dispatcher_, broadphase_, solver_, collisionConfiguration_);
	world_->setDebugDrawer(new BulletDebugDrawer(gizmos));
}

PhysicsInternal::~PhysicsInternal() {
	delete world_->getDebugDrawer();
	delete world_;

	delete solver_;
	delete collisionConfiguration_;
	delete dispatcher_;
	delete broadphase_;
}

void PhysicsInternal::FixedUpdate(float deltaTime) {
	world_->stepSimulation(deltaTime);

	// The best way to determine if collisions happened between existing objects in the world
	// is to iterate over all contact manifolds.
	int numManifolds = world_->getDispatcher()->getNumManifolds();

	// Enumerate all manifolds.
	// A contact manifold is a cache that contains all contact points between pairs of collision objects.
	for (int i = 0; i < numManifolds; ++i) {
		// Take each manifold object from the internal manifolds array by index.
		btPersistentManifold* contactManifold = world_->getDispatcher()->getManifoldByIndexInternal(i);

		// Get the number of contacts and check that there is at least one contact between the pair of bodies.
		int numContacts = contactManifold->getNumContacts();

		if (numContacts > 0) {
			const btCollisionObject* body0 = contactManifold->getBody0();
			const btCollisionObject* body1 = contactManifold->getBody1();
		}
	}

	if (debugDrawEnabled_) {
		world_->debugDrawWorld();
	}
}

bool PhysicsInternal::Raycast(const Ray& ray, float maxDistance, RaycastHit* hitInfo) {
	btCollisionWorld::ClosestRayResultCallback callback(btConvert(ray.GetOrigin()), btConvert(ray.GetPoint(maxDistance)));
	world_->rayTest(btConvert(ray.GetOrigin()), btConvert(ray.GetPoint(maxDistance)), callback);
	if (!callback.hasHit()) {
		return false;
	}

	RigidbodyInternal* rigidbody = (RigidbodyInternal*)callback.m_collisionObject->getUserPointer();
	if (rigidbody != nullptr && hitInfo != nullptr) {
		hitInfo->point = btConvert(callback.m_hitPointWorld);
		hitInfo->normal = btConvert(callback.m_hitNormalWorld);

		hitInfo->gameObject = rigidbody->GetGameObject();
	}

	return true;
}
