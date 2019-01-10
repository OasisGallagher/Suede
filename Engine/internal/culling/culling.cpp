#include "culling.h"

#include "world.h"
#include "renderer.h"
#include "profiler.h"
#include "rigidbody.h"
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
			uint64 start = Profiler::GetTimeStamp();

 			OcclusionCulling();
			World::CullingUpdate();

			Statistics::SetCullingElapsed(
				Profiler::TimeStampToSeconds(Profiler::GetTimeStamp() - start)
			);

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
	if (rigidbody->GetGameObject()->GetName() == "Rectangle212") {
		Debug::Break();
	}

	if (!rigidbody->GetGameObject()->GetActive()) {
		return false;
	}

	const Bounds& bounds = rigidbody->GetBounds();

	std::vector<glm::vec3> points;
	GeometryUtility::GetCuboidCoordinates(points, bounds.center, bounds.size);

	glm::vec2 min(std::numeric_limits<float>::max()), max(std::numeric_limits<float>::lowest());

	for (int i = 0; i < points.size(); ++i) {
		glm::vec4 p = worldToClipMatrix_ * glm::vec4(points[i], 1);
		glm::vec2 p2(Math::Clamp(p.x / p.w, -1.f, 1.f), Math::Clamp(p.y / p.w, -1.f, 1.f));

		min = glm::min(min, p2);
		max = glm::max(max, p2);
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
	btDbvtBroadphase* broadphase = (btDbvtBroadphase*)btWorld()->getBroadphase();

	btVector3 planes_n[5];
	btScalar  planes_o[5];
	const float* mmp = worldToClipMatrix;
	/*GeometryUtility::CalculateFrustumPlanes((float*)planes_n, (float*)planes_o, 0, 1, worldToClipMatrix);*/
	// Extract the RIGHT clipping plane
	planes_n[0] = btVector3((btScalar)(mmp[3] - mmp[0]), (btScalar)(mmp[7] - mmp[4]), (btScalar)(mmp[11] - mmp[8]));
	// Extract the LEFT clipping plane
	planes_n[1] = btVector3((btScalar)(mmp[3] + mmp[0]), (btScalar)(mmp[7] + mmp[4]), (btScalar)(mmp[11] + mmp[8]));
	// Extract the TOP clipping plane
	planes_n[2] = btVector3((btScalar)(mmp[3] - mmp[1]), (btScalar)(mmp[7] - mmp[5]), (btScalar)(mmp[11] - mmp[9]));
	// Extract the BOTTOM clipping plane
	planes_n[3] = btVector3((btScalar)(mmp[3] + mmp[1]), (btScalar)(mmp[7] + mmp[5]), (btScalar)(mmp[11] + mmp[9]));
	// Extract the FAR clipping plane
	planes_n[4] = btVector3((btScalar)(mmp[3] - mmp[2]), (btScalar)(mmp[7] - mmp[6]), (btScalar)(mmp[11] - mmp[10]));
	//planes_n[4]	=	-dir;	//Should work well too... (don't know without normalizations... better stay coherent)

	// Extract the RIGHT clipping plane			
	planes_o[0] = (btScalar)(mmp[15] - mmp[12]);
	// Extract the LEFT clipping plane			
	planes_o[1] = (btScalar)(mmp[15] + mmp[12]);
	// Extract the TOP clipping plane			
	planes_o[2] = (btScalar)(mmp[15] - mmp[13]);
	// Extract the BOTTOM clipping plane			
	planes_o[3] = (btScalar)(mmp[15] + mmp[13]);
	// Extract the FAR clipping plane			
	planes_o[4] = (btScalar)(mmp[15] - mmp[14]);

	dbvtCulling_->m_pCollisionObjectArray = &objectsInFrustum_;
	dbvtCulling_->m_collisionFilterMask = btBroadphaseProxy::AllFilter & ~btBroadphaseProxy::SensorTrigger;	// This won't display sensors...

#ifndef OCCLUSION_CULLING_DISABLED
		occlusionBuffer_->initialize(worldToClipMatrix, occlusionBufferSize_.x, occlusionBufferSize_.y);
		memcpy(occlusionBuffer_->eye, cameraPos, sizeof(btVector3));
		dbvtCulling_->m_ocb = occlusionBuffer_;

		btDbvt::collideOCL(broadphase->m_sets[1].m_root, planes_n, planes_o, *(btVector3*)cameraForward, SUEDE_COUNTOF(planes_n), *dbvtCulling_);
		btDbvt::collideOCL(broadphase->m_sets[0].m_root, planes_n, planes_o, *(btVector3*)cameraForward, SUEDE_COUNTOF(planes_n), *dbvtCulling_);
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
