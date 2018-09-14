#pragma once
#include "plane.h"
#include "camera.h"
#include "entity.h"
#include "projector.h"
#include "geometryutility.h"
#include "containers/freelist.h"

class DecalCreater {
	struct DecalInfo {
		Decal decal;

		Texture texture;
		glm::mat4 matrix;

		MeshTopology topology;
		std::vector<uint> indexes;
		std::vector<glm::vec3> positions;
	};

	typedef free_list<DecalInfo> DecalInfoContainer;

public:
	DecalCreater();

public:
	template <class Projecters>
	void Update(Camera camera, Projecters& projectors);
	void GetDecals(std::vector<Decal>& container);

private:
	void CreateDecal(DecalInfo* info);

	bool CreateEntityDecal(Camera camera, DecalInfo& info, Entity entity, Plane planes[6]);
	bool CreateProjectorDecal(Camera camera, Projector p, std::vector<Entity>& entities, Plane planes[6]);

	bool ClampMesh(Camera camera, std::vector<glm::vec3>& triangles, Entity entity, Plane planes[6]);

private:
	Plane planes_[6];
	Material material_;

	DecalInfoContainer decalInfos_;
};

template <class Projecters>
void DecalCreater::Update(Camera camera, Projecters& projectors) {
	decalInfos_.clear();

	std::vector<Entity> entities;
	camera->GetVisibleEntities(entities);

	for (Projecters::iterator ite = projectors.begin(); ite != projectors.end(); ++ite) {
		Projector p = *ite;
		GeometryUtility::CalculateFrustumPlanes(planes_, p->GetProjectionMatrix() * p->GetTransform()->GetWorldToLocalMatrix());

		if (!CreateProjectorDecal(camera, p, entities, planes_)) {
			break;
		}
	}
}
