#include <glm/glm.hpp>
#include "btBulletDynamicsCommon.h"

typedef unsigned uint;

class Physics {
public:
	void init() {
		// You instantiate the broad phase algorithm implementation.
		// Collision detection is done in two phases : broad and narrow.
		// In the broad phase, the physics engine quickly eliminates objects that cannot collide.
		// For example, it can run a quick check to using the bounding boxes of objects,
		// eliminating those that don¡¯t collide.It then passes only a small list of objects that 
		// can collide to the narrow phase, which is much slower, since it checks actual shapes for collision.
		// Bullet has several built - in implementations of the broad phase.In this tutorial, 
		// you¡¯re using the dynamic AABB tree implementation ¨C i.e.btDbvtBroadphase.
		broadphase_ = new btDbvtBroadphase();

		// collisionConfiguration is responsible for full, not broad, collision detection.
		// In other words, this is where the more fine - grained and accurate collision detection code runs.
		// You could create your own implementation, but for now you¡¯re using the built - in configuration again.
		collisionConfiguration_ = new btDefaultCollisionConfiguration();
		dispatcher_ = new btCollisionDispatcher(collisionConfiguration_);

		// This is what causes the objects to interact properly, taking into account gravity, game logic supplied forces,
		// collisions, and hinge constraints. It does a good job as long as you don¡¯t push it to extremes, and is one of 
		// the bottlenecks in any high performance simulation. There are parallel versions available for some threading models.
		solver_ = new btSequentialImpulseConstraintSolver();

		world_ = new btDiscreteDynamicsWorld(dispatcher_, broadphase_, solver_, collisionConfiguration_);
		world_->setGravity(btVector3(0, -0.8f, 0));
	}

	void addRigidbody(btRigidBody* body) {
		world_->addRigidBody(body);
	}

	void update() {
		float fixedDeltaTime = 0;
		world_->stepSimulation(fixedDeltaTime);
	}

	void release() {
		delete world_;
		delete solver_;
		delete collisionConfiguration_;
		delete dispatcher_;
		delete broadphase_;
	}

private:
	btBroadphaseInterface* broadphase_ = nullptr;
	btDefaultCollisionConfiguration* collisionConfiguration_ = nullptr;
	btCollisionDispatcher* dispatcher_ = nullptr;
	btSequentialImpulseConstraintSolver* solver_ = nullptr;
	btDiscreteDynamicsWorld* world_ = nullptr;
};
