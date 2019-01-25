#include "physicsinternal.h"

#include "time2.h"
#include "gizmos.h"
#include "engine.h"
#include "profiler.h"
#include "rigidbody.h"
#include "statistics.h"
#include "memory/memory.h"
#include "rigidbodyinternal.h"

void BulletDebugDrawer::drawLine(const btVector3& from, const btVector3& to, const btVector3& color) {
	Color oldColor = Gizmos::GetColor();
	Gizmos::SetColor(Color(color.x(), color.y(), color.z()));
	Gizmos::DrawLines({ btConvert(from), btConvert(to) });
	Gizmos::SetColor(oldColor);
}

void BulletDebugDrawer::drawContactPoint(const btVector3& PointOnB, const btVector3& normalOnB, btScalar distance, int lifeTime, const btVector3& color) {
}

void BulletDebugDrawer::reportErrorWarning(const char* warningString) {
	Debug::LogError(warningString);
}

Physics::Physics() : singleton2<Physics>(MEMORY_NEW(PhysicsInternal), Memory::DeleteRaw<PhysicsInternal>) {}
bool Physics::Raycast(const Ray& ray, float maxDistance, uint layerMask, RaycastHit* hitInfo) { return _suede_dinstance()->Raycast(ray, maxDistance, layerMask, hitInfo); }
void Physics::SetGravity(const glm::vec3& value) { _suede_dinstance()->SetGravity(value); }
glm::vec3 Physics::GetGravity() { return _suede_dinstance()->GetGravity(); }
void Physics::SetDebugDrawEnabled(bool value) { _suede_dinstance()->SetDebugDrawEnabled(value); }
bool Physics::GetDebugDrawEnabled() { return _suede_dinstance()->GetDebugDrawEnabled(); }

btDiscreteDynamicsWorld* PhysicsInternal::world_;

PhysicsInternal::PhysicsInternal() : debugDrawEnabled_(false) {
	// You instantiate the broad phase algorithm implementation.
	// Collision detection is done in two phases : broad and narrow.
	// In the broad phase, the physics engine quickly eliminates objects that cannot collide.
	// For example, it can run a quick check to using the bounding boxes of objects,
	// eliminating those that don't collide.It then passes only a small list of objects that 
	// can collide to the narrow phase, which is much slower, since it checks actual shapes for collision.
	// Bullet has several built - in implementations of the broad phase.In this tutorial, 
	// you're using the dynamic AABB tree implementation - i.e.btDbvtBroadphase.
	broadphase_ = MEMORY_NEW(btDbvtBroadphase);

	// collisionConfiguration is responsible for full, not broad, collision detection.
	// In other words, this is where the more fine - grained and accurate collision detection code runs.
	// You could create your own implementation, but for now you're using the built - in configuration again.
	collisionConfiguration_ = MEMORY_NEW(btDefaultCollisionConfiguration);
	dispatcher_ = MEMORY_NEW(btCollisionDispatcher, collisionConfiguration_);

	// This is what causes the objects to interact properly, taking into account gravity, game logic supplied forces,
	// collisions, and hinge constraints. It does a good job as long as you don't push it to extremes, and is one of 
	// the bottlenecks in any high performance simulation. There are parallel versions available for some threading models.
	solver_ = MEMORY_NEW(btSequentialImpulseConstraintSolver);

	world_ = MEMORY_NEW(btDiscreteDynamicsWorld, dispatcher_, broadphase_, solver_, collisionConfiguration_);
	world_->setDebugDrawer(MEMORY_NEW(BulletDebugDrawer));

	World::AddEventListener(this);
	Engine::AddFrameEventListener(this);
}


PhysicsInternal::~PhysicsInternal() {
	World::RemoveEventListener(this);
	Engine::RemoveFrameEventListener(this);

	MEMORY_DELETE(world_->getDebugDrawer());
	MEMORY_DELETE(world_);

	MEMORY_DELETE(solver_);
	MEMORY_DELETE(collisionConfiguration_);
	MEMORY_DELETE(dispatcher_);
	MEMORY_DELETE(broadphase_);
}

void PhysicsInternal::OnFrameEnter() {
	uint64 start = Profiler::GetTimeStamp();
	FixedUpdate();
	Statistics::SetPhysicsElapsed(
		Profiler::TimeStampToSeconds(Profiler::GetTimeStamp() - start)
	);
}

void PhysicsInternal::OnWorldEvent(WorldEventBasePtr e) {
	switch (e->GetEventType()) {
		case WorldEventType::GameObjectComponentChanged:
			OnGameObjectComponentChanged(std::static_pointer_cast<GameObjectComponentChangedEvent>(e));
			break;
	}
}

void PhysicsInternal::FixedUpdate() {
	world_->stepSimulation(Time::GetFixedDeltaTime());
	/*
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
	*/
}

bool PhysicsInternal::Raycast(const Ray& ray, float maxDistance, uint layerMask, RaycastHit* hitInfo) {
	btCollisionWorld::ClosestRayResultCallback callback(btConvert(ray.GetOrigin()), btConvert(ray.GetPoint(maxDistance)));
	world_->rayTest(btConvert(ray.GetOrigin()), btConvert(ray.GetPoint(maxDistance)), callback);
	if (!callback.hasHit()) {
		return false;
	}

	IRigidbody* rigidbody = (IRigidbody*)callback.m_collisionObject->getUserPointer();
	if ((rigidbody->GetGameObject()->GetLayer() & layerMask) == 0) {
		return false;
	}

	if (rigidbody != nullptr && hitInfo != nullptr) {
		hitInfo->point = btConvert(callback.m_hitPointWorld);
		hitInfo->normal = btConvert(callback.m_hitNormalWorld);

		hitInfo->gameObject = rigidbody->GetGameObject();
	}

	return true;
}

void PhysicsInternal::OnGameObjectComponentChanged(GameObjectComponentChangedEventPtr e) {
}
