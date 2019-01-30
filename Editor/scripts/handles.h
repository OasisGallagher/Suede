#pragma once
#include <vector>

#include <QObject>

#include "mesh.h"
#include "physics.h"
#include "material.h"
#include "transform.h"
#include "behaviour.h"

#include "tools/enum.h"

BETTER_ENUM(HandlesMode, int,
	Move,
	Rotate,
	Scale
)

class Handles : public QObject, public IBehaviour {
	Q_OBJECT
	SUEDE_DECLARE_COMPONENT()

public:
	virtual void Awake();
	virtual void Update();
	virtual void OnDestroy();
	virtual void OnPreRender();

public:
	void SetMode(HandlesMode value);
	HandlesMode GetMode() { return mode_; }

private:
	void Initialize();

	void InitializeMeshes();
	void InitializeMaterials();

	void SetupAxises();
	void UpdateCurrentAxis();

	void SetHandlesMesh(Mesh storage); 
	bool RaycastUnderCursor(RaycastHit& hitInfo);
	
	glm::vec3 FindAxis(GameObject current);
	glm::vec3 Project(const glm::vec3& axis, const glm::ivec2& delta);

	void InitializeMoveHandlesMesh(Mesh mesh);
	void InitializeRotateHandlesMesh(Mesh mesh);
	void InitializeScaleHandlesMesh(Mesh mesh);

	typedef void (Handles::*Handler)(const glm::vec3&, const glm::ivec2&, const glm::ivec2&);

	void MoveHandles(const glm::vec3& axis, const glm::ivec2& mousePos, const glm::ivec2& oldPos);
	void RotateHandles(const glm::vec3& axis, const glm::ivec2& mousePos, const glm::ivec2& oldPos);
	void ScaleHandles(const glm::vec3& axis, const glm::ivec2& mousePos, const glm::ivec2& oldPos);

private:
	Color color_;
	glm::vec3 tangent_;
	glm::ivec2 screenPos_;
	glm::vec3 collisionPos_;

	glm::vec3 axis_;
	GameObject current_;

	Handler handler_;
	GameObject handles_;
	HandlesMode mode_;

	enum {
		AxisCount = 3,
		Resolution = 27,
	};

	static Mesh s_meshes[AxisCount];
	static Material s_materials[AxisCount];
};
