#pragma once
#include "occlusionbuffer.h"

struct	DBVTCulling : btDbvt::ICollide {
	std::vector<btCollisionObject*>* m_pCollisionObjectArray;
	short int m_collisionFilterMask;
	btCollisionObject* m_additionalCollisionObjectToExclude;
	OcclusionBuffer* m_ocb;

	DBVTCulling(std::vector<btCollisionObject*>* _pArray = NULL);

	bool Descent(const btDbvtNode* node);

	void Process(const btDbvtNode* leaf);
	void Process(const btDbvtNode* node, btScalar depth) { Process(node); }

private:
	bool IsOccluder(const btCollisionObject* o);
};

inline bool DBVTCulling::IsOccluder(const btCollisionObject* o) {
	return (o && (o->getCollisionFlags() & btCollisionObject::CF_OCCLUDER_OBJECT));
}
