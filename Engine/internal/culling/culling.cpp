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
	, stopped_(false), occlusionCullingEnabled_(true), occlusionBufferSize_(128, 128) {
	dbvtCulling_ = MEMORY_NEW(DBVTCulling);
	occlusionBuffer_ = MEMORY_NEW(OcclusionBuffer);
}

Culling::~Culling() {
	MEMORY_DELETE(dbvtCulling_);
	MEMORY_DELETE(occlusionBuffer_);
}

void Culling::run() {
	for (; !stopped_;) {
		if (working_) {
			gameObjects_.clear();
			uint64 start = Profiler::GetTimeStamp();

			OcclusionCulling();

			for (btCollisionObject* object : *dbvtCulling_->m_pCollisionObjectArray) {
				IRigidbody* body = (IRigidbody*)object->getUserPointer();
				gameObjects_.push_back(body->GetGameObject());
			}

			float seconds = (float)Profiler::TimeStampToSeconds(Profiler::GetTimeStamp() - start);

			World::CullingUpdate();

			//World::WalkGameObjectHierarchy(this);
			listener_->OnCullingFinished();

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
	occlusionBuffer_->bufferImage(texels.data);
}

void Culling::Stop() {
	stopped_ = true;
	cond_.broadcast();
}

void Culling::Cull(const glm::mat4& worldToClipMatrix) {
	if (!working_) {
		worldToClipMatrix_ = worldToClipMatrix;
		working_ = true;
		cond_.signal();
	}
}

WalkCommand Culling::OnWalkGameObject(GameObject go) {
	if (!IsVisible(go, worldToClipMatrix_)) {
		return WalkCommand::Continue;
	}

	if (!go->GetActive()) {
		return WalkCommand::Next;
	}

	if (go->GetComponent<Renderer>() && go->GetComponent<MeshProvider>()) {
		gameObjects_.push_back(go);
	}

	return WalkCommand::Continue;
}

bool Culling::IsVisible(GameObject go, const glm::mat4& worldToClipMatrix) {
	const Bounds& bounds = go->GetBounds();
	if (bounds.IsEmpty()) {
		return false;
	}

	return FrustumCulling(bounds, worldToClipMatrix);
}

bool Culling::FrustumCulling(const Bounds& bounds, const glm::mat4& worldToClipMatrix) {
	glm::ivec2 outx, outy, outz;

	std::vector<glm::vec3> points;
	GeometryUtility::GetCuboidCoordinates(points, bounds.center, bounds.size);

	glm::vec2 min(std::numeric_limits<float>::max()), max(std::numeric_limits<float>::lowest());

	for (int i = 0; i < points.size(); ++i) {
		glm::vec4 p = worldToClipMatrix * glm::vec4(points[i], 1);

		// Note that the frustum culling(clipping) is performed in the clip coordinates,
		// just before dividing.

		if (p.x < -p.w) { ++outx.x; }
		else if (p.x > p.w) { ++outx.y; }

		if (p.y < -p.w) { ++outy.x; }
		else if (p.y > p.w) { ++outy.y; }

		if (p.z < -p.w) { ++outz.x; }
		else if (p.z > p.w) { ++outz.y; }

		glm::vec2 p2(Math::Clamp(p.x / p.w, -1.f, 1.f), Math::Clamp(p.y / p.w, -1.f, 1.f));

		min = glm::min(min, p2);
		max = glm::max(max, p2);
	}

	if (outx.x == 8 || outx.y == 8 || outy.x == 8 || outy.y == 8 || outz.x == 8 || outz.y == 8) {
		return false;
	}

	glm::vec2 size(max - min);
	return glm::dot(size, size) > MIN_NDC_RADIUS_SQUARED;
}

void Culling::Matrix16Multiply(const float* a, const float* b, float* res) {
	res[0] = a[0] * b[0] + a[1] * b[4] + a[2] * b[8] + a[3] * b[12];
	res[1] = a[0] * b[1] + a[1] * b[5] + a[2] * b[9] + a[3] * b[13];
	res[2] = a[0] * b[2] + a[1] * b[6] + a[2] * b[10] + a[3] * b[14];
	res[3] = a[0] * b[3] + a[1] * b[7] + a[2] * b[11] + a[3] * b[15];

	res[4] = a[4] * b[0] + a[5] * b[4] + a[6] * b[8] + a[7] * b[12];
	res[5] = a[4] * b[1] + a[5] * b[5] + a[6] * b[9] + a[7] * b[13];
	res[6] = a[4] * b[2] + a[5] * b[6] + a[6] * b[10] + a[7] * b[14];
	res[7] = a[4] * b[3] + a[5] * b[7] + a[6] * b[11] + a[7] * b[15];

	res[8] = a[8] * b[0] + a[9] * b[4] + a[10] * b[8] + a[11] * b[12];
	res[9] = a[8] * b[1] + a[9] * b[5] + a[10] * b[9] + a[11] * b[13];
	res[10] = a[8] * b[2] + a[9] * b[6] + a[10] * b[10] + a[11] * b[14];
	res[11] = a[8] * b[3] + a[9] * b[7] + a[10] * b[11] + a[11] * b[15];

	res[12] = a[12] * b[0] + a[13] * b[4] + a[14] * b[8] + a[15] * b[12];
	res[13] = a[12] * b[1] + a[13] * b[5] + a[14] * b[9] + a[15] * b[13];
	res[14] = a[12] * b[2] + a[13] * b[6] + a[14] * b[10] + a[15] * b[14];
	res[15] = a[12] * b[3] + a[13] * b[7] + a[14] * b[11] + a[15] * b[15];
}

void Culling::OcclusionCulling() {
	objectsInFrustum_.resize(0);	// clear() is probably slower
	Camera main = CameraUtility::GetMain();
	if (!btWorld() || !main) return;

	btDbvtBroadphase* broadphase = (btDbvtBroadphase*)btWorld()->getBroadphase();

	if (!occlusionCullingEnabled_) {
		return;
	}

	// Perform Dbvt Frustum Culling
	// Mapping stuff:---------------------------------------------------------------------------
	const btVector3& eye = btConvert(main->GetTransform()->GetPosition());
	const btVector3	dir = btConvert(main->GetTransform()->GetForward());
	const float frustumFar = main->GetFarClipPlane();

	// End mapping stuff--------------------------------------------------------------------------

	// Storage stuff------------------
	btVector3 planes_n[5];
	btScalar  planes_o[5];
	const btScalar farplane = frustumFar;
	static const int nplanes = sizeof(planes_n) / sizeof(planes_n[0]);
	static const bool cullFarPlane = true;//false;	// This can be tweaked.			
	const int	acplanes = cullFarPlane ? 5 : 4;
	// End storage stuff---------------

// Frustum plane extraction--------
// This is shorter and cleaner than the approach used in the CDTestFramework
// To better understand this approach please browse the web for a pdf named: "Fast Extraction of Viewing Frustum Planes from the World-View-Projection Matrix".
	glm::mat4 mvp = main->GetProjectionMatrix() * main->GetTransform()->GetWorldToLocalMatrix();
	const float* mmp = (const float*)&mvp;
	// Now we axtract the planes from mmp:

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

	/*
	Compared to the pdf paper named: "Fast Extraction of Viewing Frustum Planes from the World-View-Projection Matrix":
	Here, for example, the first coefficient of the RIGHT plane is (mmp[ 3]-mmp[ 0]), that in the paper is referenced as (m41-m11), since my matrices are stored in column major order and their index range is in [0,3].
	Here, for example, the offset for the right plane is (mmp[15]-mmp[12]), and in the paper is (m44-m14).
	Here the plane equations are in the form: planes_n[i].x()*x + planes_n[i].y()*y + planes_n[i].z()*z + planes_o[i] = 0
	According to the paper, plane normalization isn't needed if we just want to test if a point is inside or outside the plane, so we don't normalize them.
	Good paper, after all. The main drawback is that the matrix elements here have a (very) different convention. It should be handy to fully rewrite it with this convention...
	*/

	dbvtCulling_->m_pCollisionObjectArray = &objectsInFrustum_;
	dbvtCulling_->m_collisionFilterMask = btBroadphaseProxy::AllFilter & ~btBroadphaseProxy::SensorTrigger;	// This won't display sensors...
	dbvtCulling_->m_additionalCollisionObjectToExclude = NULL;//btCamera;

	int total = btWorld()->getCollisionObjectArray().size();

	if (occlusionCullingEnabled_) {
		occlusionBuffer_->initialize(mmp, occlusionBufferSize_.x, occlusionBufferSize_.y);
		occlusionBuffer_->eye = eye;
		dbvtCulling_->m_ocb = occlusionBuffer_;

		btDbvt::collideOCL(broadphase->m_sets[1].m_root, planes_n, planes_o, dir, acplanes, *dbvtCulling_);
		btDbvt::collideOCL(broadphase->m_sets[0].m_root, planes_n, planes_o, dir, acplanes, *dbvtCulling_);
		// btDbvt::collideOCL(root,normals,offsets,sortaxis,count,icollide): 
		// same of btDbvt::collideKDOP but with leaves sorted (min/max) along 'sortaxis'.
	}
	else {
		dbvtCulling_->m_ocb = NULL;
		btDbvt::collideKDOP(broadphase->m_sets[1].m_root, planes_n, planes_o, acplanes, *dbvtCulling_);
		btDbvt::collideKDOP(broadphase->m_sets[0].m_root, planes_n, planes_o, acplanes, *dbvtCulling_);
		// btDbvt::collideKDOP(root,normals,offsets,count,icollide): 
		// traverse the tree and call ICollide::Process(node) for all leaves located inside/on a set of planes.
	}
}
