#pragma once
#include "plane.h"
#include "camera.h"
#include "projector.h"
#include "gameobject.h"
#include "geometryutility.h"
#include "containers/freelist.h"

class DecalCreater {
	struct DecalInfo {
		Decal decal;

		Matrix4 matrix;
		ref_ptr<Texture> texture;

		MeshTopology topology;
		std::vector<uint> indexes;
		std::vector<Vector3> positions;
	};

	typedef free_list<DecalInfo> DecalInfoContainer;

public:
	DecalCreater();

public:
	template <class Projecters>
	void Update(Camera* camera, Projecters& projectors);
	void GetDecals(std::vector<Decal>& container);

private:
	void CreateDecal(DecalInfo* info);

	bool CreateGameObjectDecal(Camera* camera, DecalInfo& info, GameObject* go, Plane planes[6]);
	bool CreateProjectorDecal(Camera* camera, Projector* p, std::vector<GameObject*>& gameObjects, Plane planes[6]);

	bool ClampMesh(Camera* camera, std::vector<Vector3>& triangles, GameObject* go, Plane planes[6]);

private:
	Plane planes_[6];
	ref_ptr<Material> material_;

	DecalInfoContainer decalInfos_;
};

template <class Projecters>
void DecalCreater::Update(Camera* camera, Projecters& projectors) {
	decalInfos_.clear();

	std::vector<GameObject*> gameObjects;
	camera->GetVisibleGameObjects(gameObjects);

	for (Projecters::iterator ite = projectors.begin(); ite != projectors.end(); ++ite) {
		const ref_ptr<Projector>& p = *ite;
		GeometryUtility::CalculateFrustumPlanes(planes_, p->GetProjectionMatrix() * p->GetTransform()->GetWorldToLocalMatrix());

		if (!CreateProjectorDecal(camera, p.get(), gameObjects, planes_)) {
			break;
		}
	}
}
