#pragma once
#include <vector>

#include <QObject>

#include "mesh.h"
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

	virtual void OnPreRender();

public:
	void SetMode(HandlesMode value);
	HandlesMode GetMode() const { return mode_; }

private:
	void Initialize();
	void UpdateCurrentAxis();

	glm::vec3 FindAxis(GameObject current);
	glm::vec3 Project(const glm::vec3& axis, glm::ivec2 delta);

	void SetupAxises();
	void InitializeMaterials(Material* materials);

	void SetHandlesMesh(Mesh storage);

	void MoveHandles(const glm::vec3& axis, const glm::ivec2& mousePos, const glm::ivec2& oldPos);
	void RotateHandles(const glm::vec3& axis, const glm::ivec2& mousePos, const glm::ivec2& oldPos);

	void ScaleHandles(const glm::vec3& axis, const glm::ivec2& mousePos, const glm::ivec2& oldPos);

	void InitializeMoveHandlesMesh(Mesh mesh);
	void InitializeRotateHandlesMesh(Mesh mesh);
	void InitializeScaleHandlesMesh(Mesh mesh);

private:
	HandlesMode mode_;
	void (Handles::*method_)(const glm::vec3&, const glm::ivec2&, const glm::ivec2&);

	Mesh meshes_[HandlesMode::size()];

	Color color_;
	glm::ivec2 pos_;

	glm::vec3 axis_;
	GameObject current_;

	static GameObject handles_;
};
