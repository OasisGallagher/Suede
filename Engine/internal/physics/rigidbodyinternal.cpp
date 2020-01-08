#include "rigidbodyinternal.h"

#include "mathconvert.h"
#include "physicsinternal.h"

Rigidbody::Rigidbody() : Component(new RigidbodyInternal) {}
void Rigidbody::ShowCollisionShape(bool value) { _suede_dptr()->ShowCollisionShape(value); }
void Rigidbody::SetMass(float value) { _suede_dptr()->SetMass(value); }
float Rigidbody::GetMass() const { return _suede_dptr()->GetMass(); }
void Rigidbody::SetVelocity(const Vector3& value) { _suede_dptr()->SetVelocity(value); }
Vector3 Rigidbody::GetVelocity() const { return _suede_dptr()->GetVelocity(); }

SUEDE_DEFINE_COMPONENT_INTERNAL(Rigidbody, Component)

#define btWorld()	PhysicsInternal::btWorld()

RigidbodyInternal::RigidbodyInternal()
	: ComponentInternal(ObjectType::Rigidbody)
	, mass_(0), shapeState_(Normal), body_(nullptr), mesh_(nullptr), shape_(nullptr), showCollisionShape_(false) {
	CreateBody();
}

RigidbodyInternal::~RigidbodyInternal() {
	DestroyBody();
	DestroyShape();
}

void RigidbodyInternal::Awake() {
}

// SUEDE TODO: FixedUpdate.
void RigidbodyInternal::Update(float deltaTime) {
	return;
	if (shapeState_ != Normal) {
		if (shapeState_ != InvalidShape || RebuildShape()) {
			UpdateBody(true);
			ApplyGameObjectTransform();
			//UpdateBounds();
		}

		shapeState_ = Normal;
	}

	// SUEDE TODO: empty body.
	if (body_ != nullptr && (body_->getCollisionFlags() & btCollisionObject::CF_STATIC_OBJECT) == 0) {
		ApplyPhysicsTransform();
	}

	if (showCollisionShape_) {
		btWorld()->debugDrawObject(body_->getWorldTransform(), shape_, btVector3(1, 0, 0));
	}
}

void RigidbodyInternal::OnMessage(int messageID, void* parameter) {
	if (messageID == GameObjectMessageMeshModified) {
		shapeState_ = InvalidShape;
	}
	else if( messageID == GameObjectMessageLocalToWorldMatrixModified) {
		shapeState_ = InvalidBody;
	}
}

void RigidbodyInternal::SetMass(float value) {
	if (!Mathf::Approximately(mass_, value)) {
		mass_ = value;
		UpdateBody(false);
	}
}

//void RigidbodyInternal::UpdateBounds() {
//	btVector3 minAabb, maxAabb;
//	shape_->getAabb(body_->getWorldTransform(), minAabb, maxAabb);
//	btVector3 contactThreshold(gContactBreakingThreshold, gContactBreakingThreshold, gContactBreakingThreshold);
//	minAabb -= contactThreshold;
//	maxAabb += contactThreshold;
//	
//	if (btWorld()->getDispatchInfo().m_useContinuous && body_->getInternalType() == btCollisionObject::CO_RIGID_BODY && !body_->isStaticOrKinematicObject()) {
//		btVector3 minAabb2, maxAabb2;
//		shape_->getAabb(body_->getInterpolationWorldTransform(), minAabb2, maxAabb2);
//
//		minAabb2 -= contactThreshold;
//		maxAabb2 += contactThreshold;
//
//		minAabb.setMin(minAabb2);
//		maxAabb.setMax(maxAabb2);
//	}
//
//	bounds_.SetMinMax(btConvert(minAabb), btConvert(maxAabb));
//	GetGameObject()->RecalculateBounds(RecalculateBoundsFlagsSelf | RecalculateBoundsFlagsParent);
//}

void RigidbodyInternal::SetVelocity(const Vector3& value) {
	body_->setLinearVelocity(btConvert(value));
}

Vector3 RigidbodyInternal::GetVelocity() const {
	return btConvert(body_->getLinearVelocity());
}

bool RigidbodyInternal::RebuildShape() {
	DestroyShape();

	MeshProvider* mp = GetGameObject()->GetComponent<MeshProvider>();
	if (!mp || !mp->GetMesh() || mp->GetMesh()->GetSubMeshCount() == 0) {
		return false;
	}

	return CreateShapeFromMesh(mp->GetMesh(), GetTransform()->GetScale());
}

bool RigidbodyInternal::CreateShapeFromMesh(Mesh* mesh, const Vector3& scale) {
	SUEDE_ASSERT(shape_ == nullptr);

	// In case of a convex object, you use btConvexHullShape.
	// This class allows you to add all points of the object and uses them to automatically create the minimum convex hull for it.
	//bool convex = false;
	//if (convex) {
	//	shape_ = new btConvexHullShape;
	//	for (int i = 0; i < nindices; ++i) {
	//		((btConvexHullShape*)shape_)->addPoint(MathConvert(vertices[indices[i]]));
	//	}
	//}
	//else {
		// In case of a concave object, you use a more complicated class called btBvhTriangleMeshShape.
		// This class requires the creation of a mesh object consisting of triangles. In this step,
		// you gather triangles by grouping vertices from the list of vertices. 
		// Then you create a mesh and create a shape object from this mesh.
	btTriangleIndexVertexArray* indexedMesh = new btTriangleIndexVertexArray;

	const uint* indexes = mesh->GetGeometry()->GetIndexes();
	const Vector3* vertices = mesh->GetGeometry()->GetVertices();

	btIndexedMesh indexedSubMesh;
	for(int i = 0; i < mesh->GetSubMeshCount(); ++i) {
		SubMesh* subMesh = mesh->GetSubMesh(i);
		const TriangleBias& bias = subMesh->GetTriangleBias();
		indexedSubMesh.m_numTriangles = bias.indexCount / 3;
		indexedSubMesh.m_triangleIndexBase = (const uchar*)(bias.baseIndex + indexes);
		indexedSubMesh.m_triangleIndexStride = sizeof(uint) * 3;

		indexedSubMesh.m_numVertices = bias.indexCount;
		indexedSubMesh.m_vertexBase = (const uchar*)(bias.baseVertex + vertices);
		indexedSubMesh.m_vertexStride = sizeof(Vector3);

		indexedMesh->addIndexedMesh(indexedSubMesh);
	}

	mesh_ = indexedMesh;
	shape_ = new btBvhTriangleMeshShape(mesh_, true);
	shape_->setLocalScaling(btConvert(scale));
	//}
	return true;
}

void RigidbodyInternal::DestroyShape() {
	if (mesh_ != nullptr) {
		delete mesh_;
		mesh_ = nullptr;
	}

	if (shape_ != nullptr) {
		delete shape_;
		shape_ = nullptr;
	}
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
	// You set the mass and inertia values for the shape. You don＊t have to calculate inertia for your shape manually.
	// Instead you are using a utility function that takes a reference, btVector3, and sets the correct inertia value using the shape＊s data.
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
	// You don＊t have to use motion states to set/get the position and rotation of the object, but doing so will
	// get you several benefits, including interpolation and callbacks.
	btDefaultMotionState* motionState = new btDefaultMotionState;
	btRigidBody::btRigidBodyConstructionInfo bodyCI(mass_, motionState, nullptr);

	// bodyCI.m_restitution sets an object＊s bounciness. Imagine dropping a ball 每 a sphere 每 to the floor:
	// - With a value of 0.0, it doesn＊t bounce at all. The sphere will stick to the floor on the first touch.
	// - With a value between 0 and 1, the object bounces, but with each bounce loses part of its energy. 
	//   The sphere will bounce several times, each time lower than the previous bounce until finally it stops.
	// - With a value of more than 1, the object gains energy with each bounce. This is not very realistic, 
	//   or at least I can＊t think of a real-life object that behaves this way. Your sphere will bounce higher
	//   than the point from which it was dropped, then it will bounce even higher and so on, until it bounces right into space.
	bodyCI.m_restitution = 1.f;

	// Setting the property bodyCI.m_friction to non-zero will make the ball spin, just like it would in real life 
	// if you launch the ball into a wall at some angle other than 90 degrees.
	bodyCI.m_friction = 0.5f;

	body_ = new btRigidBody(bodyCI);

	// This is important moment. Sometimes you only have access to a physics body 每 for example, 
	// when Bullet calls your callback and passes you the body 每 but you want to get the node object that holds this body.
	// In this line, you＊re making that possible.
	body_->setUserPointer(this);

	// You＊re limiting object movement to a 2D plane (x,y).
	// This keeps your ball and other objects from bouncing somewhere along the z-axis.
	// body->setLinearFactor(btVector3(1, 1, 0));
	body_->setLinearFactor(btVector3(1, 1, 1));
}

void RigidbodyInternal::DestroyBody() {
	if (body_ != nullptr) {
		btWorld()->removeRigidBody(body_);
		delete body_->getMotionState();
		delete body_;
	}
}
