#include "rigidbodyinternal.h"

#include "mathconvert.h"

IRigidbody::IRigidbody() : IComponent(MEMORY_NEW(RigidbodyInternal)) {}
void IRigidbody::SetMass(float value) { _suede_dptr()->SetMass(value); }
float IRigidbody::GetMass() const { return _suede_dptr()->GetMass(); }
void IRigidbody::SetVelocity(const glm::vec3& value) { _suede_dptr()->SetVelocity(value); }
glm::vec3 IRigidbody::GetVelocity() const { return _suede_dptr()->GetVelocity(); }

SUEDE_DEFINE_COMPONENT(IRigidbody, IComponent)

RigidbodyInternal::RigidbodyInternal() : ComponentInternal(ObjectType::Rigidbody), mass_(1.f), body_(nullptr), mesh_(nullptr), shape_(nullptr) {
}

RigidbodyInternal::~RigidbodyInternal() {
	DestroyBody();
	DestroyShape();
}

void RigidbodyInternal::Awake() {
	if (RebuildShape()) {
		UpdateBody();
		ApplyGameObjectTransform();
	}
}

// SUEDE TODO: FixedUpdate.
void RigidbodyInternal::Update() {
	// SUEDE TODO: empty body.
	if (body_ != nullptr && (body_->getCollisionFlags() & btCollisionObject::CF_STATIC_OBJECT) != 0) {
		ApplyPhysicsTransform();
	}
}

void RigidbodyInternal::OnMessage(int messageID, void* parameter) {
	if (messageID == GameObjectMessageMeshModified) {
		if (RebuildShape()) {
			UpdateBody();
			ApplyGameObjectTransform();
		}
	}
}

void RigidbodyInternal::SetMass(float value) {
	if (!Math::Approximately(mass_, value)) {
		mass_ = value;
		UpdateBody();
	}
}

// A transform is a translation [vector] plus a rotation [quaternion].
// A "transform" alone is usually sufficient to describe everything you need to 
// use to map a physics body [in Bullet] into your graphical rendering system.
void RigidbodyInternal::SetPosition(const glm::vec3& value) {
	btTransform transform = body_->getWorldTransform();
	transform.setOrigin(btConvert(value));
	body_->setWorldTransform(transform);
}

// It＊s time to change the getter method by implementing backwards synchronization. 
// This means when you get the position property in order to understand where you need to draw the object, 
// you get the position of the physics body and draw the object right at that location.
glm::vec3 RigidbodyInternal::GetPosition() const {
	const btVector3& pos = body_->getWorldTransform().getOrigin();
	return btConvert(pos);
}

void RigidbodyInternal::SetRotation(const glm::quat& value) {
	btTransform transform = body_->getWorldTransform();
	transform.setRotation(btConvert(value));
	body_->setWorldTransform(transform);
}

glm::quat RigidbodyInternal::GetRotation() const {
	const btQuaternion& rotation = body_->getWorldTransform().getRotation();
	return btConvert(rotation);
}

void RigidbodyInternal::SetVelocity(const glm::vec3& value) {
	body_->setLinearVelocity(btConvert(value));
}

glm::vec3 RigidbodyInternal::GetVelocity() const {
	return btConvert(body_->getLinearVelocity());
}

bool RigidbodyInternal::RebuildShape() {
	DestroyShape();

	MeshProvider mp = GetGameObject()->GetComponent<IMeshProvider>();
	if (!mp) {
		Debug::LogWarning("failed to create shape, no MeshProvider attached.");
		return false;
	}

	CreateShapeFromMesh(mp->GetMesh());
	return true;
}

void RigidbodyInternal::CreateShapeFromMesh(Mesh mesh) {
	ASSERT(shape_ == nullptr);

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

	mesh_ = indexedMesh;
	shape_ = MEMORY_NEW(btBvhTriangleMeshShape, mesh_, true);
	//}
}

void RigidbodyInternal::DestroyShape() {
	if (mesh_ != nullptr) {
		MEMORY_DELETE(mesh_);
		mesh_ = nullptr;
	}

	if (shape_ != nullptr) {
		MEMORY_DELETE(shape_);
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
	transform.setFromOpenGLMatrix((btScalar*)&GetTransform()->GetLocalToWorldMatrix());
}

void RigidbodyInternal::UpdateBody() {
	// You set the mass and inertia values for the shape. You don＊t have to calculate inertia for your shape manually.
	// Instead you are using a utility function that takes a reference, btVector3, and sets the correct inertia value using the shape＊s data.
	btVector3 intertia;
	shape_->calculateLocalInertia(mass_, intertia);

	if (body_ != nullptr) {
		body_->setMassProps(mass_, intertia);
	}
	else {
		CreateBody(mass_, intertia);
	}
}

void RigidbodyInternal::CreateBody(float mass, const btVector3& intertia) {
	ASSERT(body_ == nullptr);

	Transform transform = GetGameObject()->GetComponent<ITransform>();

	// MotionState is a convenient class that allows you to sync a physical body and with your drawable objects.
	// You don＊t have to use motion states to set/get the position and rotation of the object, but doing so will
	// get you several benefits, including interpolation and callbacks.
	btDefaultMotionState* motionState = MEMORY_NEW(btDefaultMotionState, btTransform(btConvert(transform->GetRotation()), btConvert(transform->GetPosition())));

	btRigidBody::btRigidBodyConstructionInfo bodyCI(mass, motionState, shape_, intertia);

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

	body_ = MEMORY_NEW(btRigidBody, bodyCI);

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
		MEMORY_DELETE(body_->getMotionState());
		MEMORY_DELETE(body_);
	}
}
