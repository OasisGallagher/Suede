#include "dbvtculling.h"

#include "occlusionbuffer.h"

DBVTCulling::DBVTCulling(std::vector<btCollisionObject*>* _pArray)
	: m_pCollisionObjectArray(_pArray), m_collisionFilterMask(btBroadphaseProxy::AllFilter & ~btBroadphaseProxy::SensorTrigger), m_additionalCollisionObjectToExclude(NULL), m_ocb(NULL) {
}

bool DBVTCulling::Descent(const btDbvtNode* node) {
	return(m_ocb->queryOccluder(node->volume.Center(), node->volume.Extents()));
}

void DBVTCulling::Process(const btDbvtNode* leaf) {
	btBroadphaseProxy*	proxy = static_cast <btBroadphaseProxy*> (leaf->data);
	btCollisionObject* co = static_cast <btCollisionObject*> (proxy->m_clientObject);
	if (m_ocb && IsOccluder(co) && co->getCollisionShape()) {
		static btVector3 aabbMin;
		static btVector3 aabbMax;
		co->getCollisionShape()->getAabb(btTransform::getIdentity(), aabbMin, aabbMax);	// Actually here I should get the MINIMAL aabb that can be nested INSIDE the shape (ie. only btBoxShapes work)
		m_ocb->appendOccluder((aabbMax - aabbMin)*btScalar(0.5f), co->getWorldTransform());	// Note that only a btVector3 (the inner box shape half extent) seems to be necessary here.
	}

	if ((proxy->m_collisionFilterGroup & m_collisionFilterMask) != 0 && co != m_additionalCollisionObjectToExclude) {
		m_pCollisionObjectArray->push_back(co);
	}
}
