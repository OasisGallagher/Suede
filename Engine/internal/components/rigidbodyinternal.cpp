#include "rigidbodyinternal.h"

#include "internal/physics/mathconvert.h"
#include "internal/physics/physicsinternal.h"

IRigidbody::IRigidbody() : IComponent(MEMORY_NEW(RigidbodyInternal, this)) {}
void IRigidbody::SetMass(float value) { _suede_dptr()->SetMass(value); }
float IRigidbody::GetMass() const { return _suede_dptr()->GetMass(); }
void IRigidbody::SetOccluderEnabled(bool value) { _suede_dptr()->SetOccluderEnabled(value); }
bool IRigidbody::GetOccluderEnabled() const { return _suede_dptr()->GetOccluderEnabled(); }
const Bounds& IRigidbody::GetBounds() const { return _suede_dptr()->GetBounds(); }
void IRigidbody::SetVelocity(const glm::vec3& value) { _suede_dptr()->SetVelocity(value); }
glm::vec3 IRigidbody::GetVelocity() const { return _suede_dptr()->GetVelocity(); }

SUEDE_DEFINE_COMPONENT_INTERNAL(Rigidbody, Component)

#define btWorld()	PhysicsInternal::btWorld()

RigidbodyInternal::RigidbodyInternal(IRigidbody* self)
	: ComponentInternal(self, ObjectType::Rigidbody)
	, mass_(0), shapeState_(Normal), body_(nullptr), indexedMesh_(nullptr), shape_(nullptr) {
	CreateBody();
}

RigidbodyInternal::~RigidbodyInternal() {
	OnDestroy();
}

void RigidbodyInternal::OnDestroy() {
	DestroyBody();
	DestroyShape();
}

// SUEDE TODO: FixedUpdate.
void RigidbodyInternal::Update() {
	if (shapeState_ != Normal) {
		if ((shapeState_ & InvalidShape) != 0) {
			RebuildShape();
		}

		UpdateBody(true);
		ApplyGameObjectTransform();
		UpdateBounds();

		shapeState_ = Normal;
		RequestRecalculateUpdateStrategy();
	}

	if ((body_->getCollisionFlags() & btCollisionObject::CF_STATIC_OBJECT) == 0) {
		ApplyPhysicsTransform();
	}
}

int RigidbodyInternal::GetUpdateStrategy() {
	if (shapeState_ != Normal || (body_->getCollisionFlags() & btCollisionObject::CF_STATIC_OBJECT) == 0) {
		return UpdateStrategyRendering;
	}

	return UpdateStrategyNone;
}

void RigidbodyInternal::OnMessage(int messageID, void* parameter) {
	bool needUpdate = false;
	if (messageID == GameObjectMessageMeshModified) {
		needUpdate = true;
		shapeState_ |= InvalidShape;
	}
	else if (messageID == GameObjectMessageLocalToWorldMatrixModified) {
		needUpdate = true;
		shapeState_ |= InvalidBody;
		shape_->setLocalScaling(btConvert(GetTransform()->GetScale()));
	}

	if (needUpdate) {
		RequestRecalculateUpdateStrategy();
	}
}

void RigidbodyInternal::RequestRecalculateUpdateStrategy() {
	World::FireEvent(new GameObjectUpdateStrategyChangedEvent(GetGameObject()));
	GetGameObject()->RecalculateUpdateStrategy();
}

void RigidbodyInternal::SetMass(float value) {
	if (!Math::Approximately(mass_, value)) {
		mass_ = value;
		UpdateBody(false);
	}
}

void RigidbodyInternal::UpdateBounds() {
	btVector3 minAabb, maxAabb;
	shape_->getAabb(body_->getWorldTransform(), minAabb, maxAabb);
	btVector3 contactThreshold(gContactBreakingThreshold, gContactBreakingThreshold, gContactBreakingThreshold);
	minAabb -= contactThreshold;
	maxAabb += contactThreshold;

	if (btWorld()->getDispatchInfo().m_useContinuous && body_->getInternalType() == btCollisionObject::CO_RIGID_BODY && !body_->isStaticOrKinematicObject()) {
		btVector3 minAabb2, maxAabb2;
		shape_->getAabb(body_->getInterpolationWorldTransform(), minAabb2, maxAabb2);

		minAabb2 -= contactThreshold;
		maxAabb2 += contactThreshold;

		minAabb.setMin(minAabb2);
		maxAabb.setMax(maxAabb2);
	}

	bounds_.SetMinMax(btConvert(minAabb), btConvert(maxAabb));
	GetGameObject()->RecalculateBounds(RecalculateBoundsFlagsSelf | RecalculateBoundsFlagsParent);
}

void RigidbodyInternal::SetVelocity(const glm::vec3& value) {
	body_->setLinearVelocity(btConvert(value));
}

glm::vec3 RigidbodyInternal::GetVelocity() const {
	return btConvert(body_->getLinearVelocity());
}

void RigidbodyInternal::SetOccluderEnabled(bool value) {
	body_->setCollisionFlags(
		value ? (body_->getCollisionFlags() | btCollisionObject::CF_OCCLUDER_OBJECT)
			: (body_->getCollisionFlags() & ~btCollisionObject::CF_OCCLUDER_OBJECT)
	);
}

bool RigidbodyInternal::GetOccluderEnabled() const {
	return body_ != nullptr && (body_->getCollisionFlags() & btCollisionObject::CF_OCCLUDER_OBJECT) != 0;
}

bool RigidbodyInternal::RebuildShape() {
	DestroyShape();

	MeshProvider mp = GetGameObject()->GetComponent<MeshProvider>();
	if (!mp || !mp->GetMesh() || mp->GetMesh()->GetSubMeshCount() == 0) {
		return false;
	}

	return CreateShapeFromMesh(mp->GetMesh(), GetTransform()->GetScale());
}

bool RigidbodyInternal::CreateShapeFromMesh(Mesh mesh, const glm::vec3& scale) {
	SUEDE_ASSERT(shape_ == nullptr);

	// In case of a convex object, you use btConvexHullShape.
	// This class allows you to add all points of the object and uses them to automatically create the minimum convex hull for it.
	//bool convex = false;
	//if (convex) {
	//	shape_ = MEMORY_NEW(btConvexHullShape);
	//	for (int i = 0; i < nindices; ++i) {
	//		((btConvexHullShape*)shape_)->addPoint(MathConvert(vertices[indices[i]]));
	//	}
	//}
	//else {
	// In case of a concave object, you use a more complicated class called btBvhTriangleMeshShape.
	// This class requires the creation of a mesh object consisting of triangles. In this step,
	// you gather triangles by grouping vertices from the list of vertices. 
	// Then you create a mesh and create a shape object from this mesh.
	if (mesh->GetTopology() != MeshTopology::Triangles) {
		CreateShapeFromPoints(mesh);
	}
	else {
		CreateShapeFromTriangles(mesh);
	}

	shape_->setMargin(0.1f);
	shape_->setLocalScaling(btConvert(scale));

	//}
	return true;
}

void RigidbodyInternal::CreateShapeFromPoints(Mesh mesh) {
	btConvexHullShape* convexHullShape = MEMORY_NEW(btConvexHullShape);
	const glm::vec3* vertices = mesh->MapVertices();
	uint nvertex = mesh->GetVertexCount();

	for (uint i = 0; i < nvertex; ++i) {
		convexHullShape->addPoint(btConvert(vertices[i]), false);
	}

	mesh->UnmapVertices();

	convexHullShape->optimizeConvexHull();
	convexHullShape->recalcLocalAabb();

	shape_ = convexHullShape;
}

void RigidbodyInternal::CreateShapeFromTriangles(Mesh mesh) {
	btTriangleIndexVertexArray* indexedMesh = MEMORY_NEW(btTriangleIndexVertexArray);

	const uint* indexes = mesh->MapIndexes();
	const glm::vec3* vertices = mesh->MapVertices();

	btIndexedMesh indexedSubMesh;
	for (SubMesh& subMesh : mesh->GetSubMeshes()) {
		const TriangleBias& bias = subMesh->GetTriangleBias();
		indexedSubMesh.m_numTriangles = bias.indexCount / 3;
		indexedSubMesh.m_triangleIndexBase = (const uchar*)(bias.baseIndex + indexes);
		indexedSubMesh.m_triangleIndexStride = sizeof(uint) * 3;

		indexedSubMesh.m_numVertices = bias.indexCount;
		indexedSubMesh.m_vertexBase = (const uchar*)(bias.baseVertex + vertices);
		indexedSubMesh.m_vertexStride = sizeof(glm::vec3);

		indexedMesh->addIndexedMesh(indexedSubMesh);
	}

	mesh->UnmapIndexes();
	mesh->UnmapVertices();

	indexedMesh_ = indexedMesh;

	// SUEDE TODO: slow operation.
	shape_ = MEMORY_NEW(btBvhTriangleMeshShape, indexedMesh_, true);
}

void RigidbodyInternal::DestroyShape() {
	MEMORY_DELETE(indexedMesh_); indexedMesh_ = nullptr;
	MEMORY_DELETE(shape_); shape_ = nullptr;
}

void RigidbodyInternal::ApplyPhysicsTransform() {
	const btTransform& transform = body_->getWorldTransform();
	GetTransform()->SetPosition(btConvert(transform.getOrigin()));
	GetTransform()->SetRotation(btConvert(transform.getRotation()));
}

void RigidbodyInternal::ApplyGameObjectTransform() {
	btTransform& transform = body_->getWorldTransform();
	transform.setOrigin(btConvert(GetTransform()->GetPosition()));
	transform.setRotation(btConvert(GetTransform()->GetRotation()));
	//transform.setFromOpenGLMatrix((btScalar*)&GetTransform()->GetLocalToWorldMatrix());
}

void RigidbodyInternal::UpdateBody(bool updateWorldRigidbody) {
	// You set the mass and inertia values for the shape. You don't have to calculate inertia for your shape manually.
	// Instead you are using a utility function that takes a reference, btVector3, and sets the correct inertia value using the shape's data.
	btVector3 intertia;
	shape_->calculateLocalInertia(mass_, intertia);

	body_->setCollisionShape(shape_);
	body_->setMassProps(mass_, intertia);

	if (updateWorldRigidbody) {
		btWorld()->removeRigidBody(body_);
		btWorld()->addRigidBody(body_);
	}
}

void RigidbodyInternal::CreateBody() {
	SUEDE_ASSERT(body_ == nullptr);

	// MotionState is a convenient class that allows you to sync a physical body and with your drawable objects.
	// You don't have to use motion states to set/get the position and rotation of the object, but doing so will
	// get you several benefits, including interpolation and callbacks.
	btDefaultMotionState* motionState = MEMORY_NEW(btDefaultMotionState);
	btRigidBody::btRigidBodyConstructionInfo bodyCI(mass_, motionState, nullptr);

	// bodyCI.m_restitution sets an object's bounciness. Imagine dropping a ball - a sphere - to the floor:
	// - With a value of 0.0, it doesn't bounce at all. The sphere will stick to the floor on the first touch.
	// - With a value between 0 and 1, the object bounces, but with each bounce loses part of its energy. 
	//   The sphere will bounce several times, each time lower than the previous bounce until finally it stops.
	// - With a value of more than 1, the object gains energy with each bounce. This is not very realistic, 
	//   or at least I can't think of a real-life object that behaves this way. Your sphere will bounce higher
	//   than the point from which it was dropped, then it will bounce even higher and so on, until it bounces right into space.
	bodyCI.m_restitution = 1.f;

	// Setting the property bodyCI.m_friction to non-zero will make the ball spin, just like it would in real life 
	// if you launch the ball into a wall at some angle other than 90 degrees.
	bodyCI.m_friction = 0.5f;

	body_ = MEMORY_NEW(btRigidBody, bodyCI);

	// This is important moment. Sometimes you only have access to a physics body - for example, 
	// when Bullet calls your callback and passes you the body - but you want to get the node object that holds this body.
	// In this line, you're making that possible.
	body_->setUserPointer(_suede_self());

	// You're limiting object movement to a 2D plane (x,y).
	// This keeps your ball and other objects from bouncing somewhere along the z-axis.
	// body->setLinearFactor(btVector3(1, 1, 0));
	body_->setLinearFactor(btVector3(1, 1, 1));
	body_->setCollisionFlags(body_->getCollisionFlags() | btCollisionObject::CF_OCCLUDER_OBJECT);
}

void RigidbodyInternal::DestroyBody() {
	if (body_ != nullptr) {
		btWorld()->removeRigidBody(body_);
		MEMORY_DELETE(body_->getMotionState());
		MEMORY_DELETE(body_);
		body_ = nullptr;
	}
}
