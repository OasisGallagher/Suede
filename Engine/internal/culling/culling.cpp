#include "culling.h"

#include "world.h"
#include "renderer.h"
#include "profiler.h"
#include "rigidbody.h"
#include "geometries.h"
#include "statistics.h"
#include "dbvtculling.h"
#include "memory/memory.h"
#include "geometryutility.h"
#include "internal/async/async.h"
#include "internal/base/renderdefines.h"
#include "internal/physics/physicsinternal.h"

#define btWorld()	PhysicsInternal::btWorld()

Culling::Culling(CullingListener* listener)
	: cond_(mutex_), listener_(listener), working_(false)
	, stopped_(false), occlusionBufferSize_(128, 128) {
	dbvtCulling_ = MEMORY_NEW(DBVTCulling);
	dbvtCulling_->m_checkCollisionObject = std::bind(&Culling::IsVisible, this, std::placeholders::_1);

	occlusionBuffer_ = MEMORY_NEW(OcclusionBuffer);
}

Culling::~Culling() {
	MEMORY_DELETE(dbvtCulling_);
	MEMORY_DELETE(occlusionBuffer_);
}

void Culling::run() {
	for (; !stopped_;) {
		if (working_) {
			PROFILER_RECORD(elapsed,
 				OcclusionCulling();
				World::CullingUpdate();
			);

			Statistics::SetCullingElapsed(elapsed);
			working_ = false;
		}

		ZTHREAD_LOCK_SCOPE(mutex_);
		cond_.wait();
	}
}

void Culling::GetCullingBuffer(TexelMap& texels) {
	texels.width = occlusionBuffer_->sizes[0];
	texels.height = occlusionBuffer_->sizes[1];
	texels.alignment = 4;
	texels.textureFormat = TextureFormat::Luminance;
	texels.colorStreamFormat = ColorStreamFormat::Luminance;
	occlusionBuffer_->getBufferTextureData(texels.data);
}

void Culling::Stop() {
	stopped_ = true;
	cond_.broadcast();
}

void Culling::Cull(const glm::vec3& cameraPos, const glm::vec3& cameraForward, float farClipPlane, const glm::mat4& worldToClipMatrix) {
	if (!working_) {
		cameraPos_ = cameraPos;
		cameraForward_ = cameraForward;
		farClipPlane_ = farClipPlane;
		worldToClipMatrix_ = worldToClipMatrix;

		working_ = true;
		cond_.signal();
	}
}

bool Culling::IsVisible(btCollisionObject* co) {
	Rigidbody rigidbody = (IRigidbody*)co->getUserPointer();
	if (!rigidbody->GetGameObject()->GetActive()) {
		return false;
	}

	const Bounds& bounds = rigidbody->GetBounds();

	std::vector<glm::vec3> points;
	std::vector<uint> indexes;
	Geometries::Cuboid(points, indexes, bounds.center, bounds.size);

	glm::vec2 min(std::numeric_limits<float>::max()), max(std::numeric_limits<float>::lowest());

	for (int i = 0; i < points.size(); ++i) {
		glm::vec4 p = worldToClipMatrix_ * glm::vec4(points[i], 1);
		glm::vec2 p2(Math::Clamp(p.x / p.w, -1.f, 1.f), Math::Clamp(p.y / p.w, -1.f, 1.f));

		min = Math::Min(min, p2);
		max = Math::Max(max, p2);
	}

	glm::vec2 size(max - min);
	return glm::dot(size, size) > MIN_NDC_RADIUS_SQUARED;
}

void Culling::OcclusionCulling() {
	gameObjects_.resize(0);
	objectsInFrustum_.resize(0);

	BulletDBVTCulling((float*)&cameraPos_, (float*)&cameraForward_, farClipPlane_, (float*)&worldToClipMatrix_);

	for (btCollisionObject* object : *dbvtCulling_->m_pCollisionObjectArray) {
		IRigidbody* body = (IRigidbody*)object->getUserPointer();
		gameObjects_.push_back(body->GetGameObject());
	}

	listener_->OnCullingFinished();
}

void Culling::BulletDBVTCulling(const float* cameraPos, const float* cameraForward, float farClipPlane, const float* worldToClipMatrix) {
	btVector3 planes_n[6];
	btScalar  planes_o[6];

	// According to the paper(https://www.gamedevs.org/uploads/fast-extraction-viewing-frustum-planes-from-world-view-projection-matrix.pdf), 
	// plane normalization isn't needed if we just want to test if a point is inside or outside the plane, so we don't normalize them.
	GeometryUtility::CalculateFrustumPlanes((float*)planes_n, 0, sizeof(btVector3) / sizeof(btScalar), (float*)planes_o, 0, 1, worldToClipMatrix);

	dbvtCulling_->m_pCollisionObjectArray = &objectsInFrustum_;
	dbvtCulling_->m_collisionFilterMask = btBroadphaseProxy::AllFilter & ~btBroadphaseProxy::SensorTrigger;	// This won't display sensors...

	btDbvtBroadphase* broadphase = (btDbvtBroadphase*)btWorld()->getBroadphase();

#ifndef OCCLUSION_CULLING_DISABLED
	occlusionBuffer_->initialize(worldToClipMatrix, occlusionBufferSize_.x, occlusionBufferSize_.y);
	occlusionBuffer_->eye.setValue(cameraPos[0], cameraPos[1], cameraPos[2]);
	dbvtCulling_->m_ocb = occlusionBuffer_;

	// exclude near clip plane.
	planes_n[4] = planes_n[5];
	planes_o[4] = planes_o[5];

	btDbvt::collideOCL(broadphase->m_sets[1].m_root, planes_n, planes_o, *(btVector3*)cameraForward, SUEDE_COUNTOF(planes_n) - 1, *dbvtCulling_);
	btDbvt::collideOCL(broadphase->m_sets[0].m_root, planes_n, planes_o, *(btVector3*)cameraForward, SUEDE_COUNTOF(planes_n) - 1, *dbvtCulling_);
	// btDbvt::collideOCL(root,normals,offsets,sortaxis,count,icollide): 
	// same of btDbvt::collideKDOP but with leaves sorted (min/max) along 'sortaxis'.
#else
	dbvtCulling_->m_ocb = NULL;
	btDbvt::collideKDOP(broadphase->m_sets[1].m_root, planes_n, planes_o, acplanes, *dbvtCulling_);
	btDbvt::collideKDOP(broadphase->m_sets[0].m_root, planes_n, planes_o, acplanes, *dbvtCulling_);
	// btDbvt::collideKDOP(root,normals,offsets,count,icollide): 
	// traverse the tree and call ICollide::Process(node) for all leaves located inside/on a set of planes.
#endif
}
