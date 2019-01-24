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
	virtual void OnPostRender();

private:
	void Initialize();
	void InitializeMesh();
	void InitializeMaterial();

	void CalculateHandlesGeometry(std::vector<glm::vec3>& points, std::vector<uint>& indexes);

private:
	glm::vec3 scale_;

	static Mesh mesh_;
	static Material materials_[3];
};
