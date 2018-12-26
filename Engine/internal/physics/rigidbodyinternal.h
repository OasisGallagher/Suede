#include "rigidbody.h"
#include "internal/components/componentinternal.h"

#include "bullet/btBulletDynamicsCommon.h"

enum FigureState {
	Normal,
	InvalidBody,
	InvalidShape,
};

class Figure {
public:
	Figure(void* userPointer);
	~Figure();

public:
	// SUEDE TODO: DEBUG.
	void ShowCollisionShape(bool value) { showCollisionShape_ = value; }

	void Update();
	void Invalidate(FigureState state);

	void SetMass(float value);
	float GetMass() const { return mass_; }

	void SetVelocity(const glm::vec3& value);
	glm::vec3 GetVelocity() const;

	const Bounds& GetBounds() const { return bounds_; }

private:
	void CreateBody();
	void DestroyBody();
	void UpdateBody(bool updateWorldRigidbody);

	void UpdateBounds();

	bool RebuildShape(Mesh mesh, const glm::vec3& scale);
	bool CreateShapeFromMesh(Mesh mesh, const glm::vec3& scale);
	void DestroyShape();

	void ApplyPhysicsTransform();
	void ApplyGameObjectTransform();

private:
	// mass is pretty obviously the mass of the object, but it has another not-so-obvious significance. 
	// In Bullet, you can have three types of objects, depending on their mass:
	// - The mass of a static object equals zero.These are immovable objects.In your game, the border is a static object.
	// - The mass of a kinematic object also equals zero, but you can move such objects with code by explicitly setting 
	//   their position and rotation. In your game, the paddle and bricks are kinematic objects.
	// - The mass of a dynamic object is non - zero.You move such objects by applying a force to them.In your game, 
	//   the ball is a dynamic object.You’ll set its direction and velocity and let the physics engine do the work.
	//   When the ball hits a border or a brick, it will bounce back, but it can never affect the positions of the border, 
	//   brick or paddle since they are immovable.
	float mass_;
	bool showCollisionShape_;

	Bounds bounds_;
	FigureState state_;

	// The reference to a rigid body.
	// Using this property, you’ll allow the game scene to work with the physics body of the node.
	btRigidBody* body_;

	void* userPointer_;

	// The shape of the physics body. 
	// btCollisionShape is an abstract class, and there are several different implementations of collision shapes. 
	// For example, you can describe the shape of an object as a sphere using btSphereShape or you can create 
	// complicated shapes with btBvhTriangleMeshShape, specifying vertices of triangles just like you do when rendering 
	// complex objects in OpenGL.
	btCollisionShape* shape_;
	btStridingMeshInterface* mesh_;
};

class RigidbodyInternal : public ComponentInternal {
public:
	RigidbodyInternal();
	~RigidbodyInternal();

public:
	virtual void Awake() {}

	// SUEDE TODO: FixedUpdate.
	virtual void Update() { figure_->Update(); }

	virtual int GetUpdateStrategy() { return UpdateStrategyRendering; }
	virtual void OnMessage(int messageID, void* parameter);

public:
	// SUEDE TODO: DEBUG.
	void ShowCollisionShape(bool value) { figure_->ShowCollisionShape(value); }

	void SetMass(float value) { figure_->SetMass(value); }
	float GetMass() const { return figure_->GetMass(); }

	void SetVelocity(const glm::vec3& value) { figure_->SetVelocity(value); }
	glm::vec3 GetVelocity() const { return figure_->GetVelocity(); }

	const Bounds& GetBounds() const { return figure_->GetBounds(); }

private:
	Figure* figure_;
};
