#include "dbvtculling.h"

#include "occlusionbuffer.h"

bool DBVTCulling::Descent(const btDbvtNode* node) {
	return(m_ocb->queryOccluder(node->volume.Center(), node->volume.Extents()));
}

void DBVTCulling::Process(const btDbvtNode* leaf) {
	btBroadphaseProxy* proxy = (btBroadphaseProxy*)leaf->data;
	btCollisionObject* collisionObject = (btCollisionObject*)proxy->m_clientObject;

	if (!m_checkCollisionObject || m_checkCollisionObject(collisionObject)) {
		applyOccluder(collisionObject, proxy);
	}
}

void DBVTCulling::applyOccluder(btCollisionObject* collisionObject, btBroadphaseProxy* proxy) {
	if (m_ocb && isOccluder(collisionObject) && collisionObject->getCollisionShape()) {
		static btVector3 aabbMin;
		static btVector3 aabbMax;

		// Actually here I should get the MINIMAL aabb that can be nested INSIDE the shape (ie. only btBoxShapes work)
		collisionObject->getCollisionShape()->getAabb(btTransform::getIdentity(), aabbMin, aabbMax);

		// Note that only a btVector3 (the inner box shape half extent) seems to be necessary here.
		m_ocb->appendOccluder((aabbMax - aabbMin)*btScalar(0.5f), collisionObject->getWorldTransform());
	}

	if ((proxy->m_collisionFilterGroup & m_collisionFilterMask) != 0) {
		m_pCollisionObjectArray->push_back(collisionObject);
	}
}
