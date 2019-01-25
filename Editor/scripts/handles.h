#pragma once
#include <vector>

#include <QObject>

#include "mesh.h"
#include "material.h"
#include "transform.h"
#include "behaviour.h"

class Handles : public QObject, public IBehaviour {
	Q_OBJECT
	SUEDE_DECLARE_COMPONENT()

public:
	virtual void Awake();
	virtual void Update();

	virtual void OnPreRender();

private:
	void Initialize();
	void UpdateCurrentAxis();

	glm::vec3 FindAxis(GameObject current);

	void InitializeMaterials(Material* materials);
	void SetupAxises(Mesh storage, Material* materials);

	void MoveHandles(const glm::vec3& axis, const glm::ivec2& mousePos, glm::ivec2& oldPos);

	void CalculateHandlesGeometry(std::vector<glm::vec3>& points, std::vector<uint>& indexes);

private:
	float time_;
	Color color_;
	glm::ivec2 pos_;

	glm::vec3 axis_;
	GameObject current_;

	static GameObject handles_;
};
