#pragma once
#include <functional>
#include "occlusionbuffer.h"

struct	DBVTCulling : btDbvt::ICollide {
	OcclusionBuffer* m_ocb = NULL;
	std::vector<btCollisionObject*>* m_pCollisionObjectArray = NULL;
	int m_collisionFilterMask = (btBroadphaseProxy::AllFilter & ~btBroadphaseProxy::SensorTrigger);
	std::function<bool(btCollisionObject*)> m_checkCollisionObject;

	bool Descent(const btDbvtNode* node);

	void Process(const btDbvtNode* leaf);
	void Process(const btDbvtNode* node, btScalar depth) { Process(node); }

private:
	bool isOccluder(const btCollisionObject* o);
	void applyOccluder(btCollisionObject* collisionObject, btBroadphaseProxy* proxy);
};

inline bool DBVTCulling::isOccluder(const btCollisionObject* o) {
	return (o && (o->getCollisionFlags() & btCollisionObject::CF_OCCLUDER_OBJECT));
}
