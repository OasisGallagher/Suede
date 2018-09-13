#include "decalcreater.h"

#include "resources.h"
#include "variables.h"

bool DecalCreater::CreateEntityDecal(Camera camera, DecalInfo& info, Entity entity, Plane planes[6]) {
	std::vector<glm::vec3> triangles;
	if (!ClampMesh(camera, triangles, entity, planes)) {
		return false;
	}

	std::vector<uint> indexes;
	indexes.reserve(triangles.size());
	for (uint i = 0; i < triangles.size(); ++i) {
		indexes.push_back(i);
	}

	info.indexes = indexes;
	info.positions = triangles;
	info.topology = MeshTopology::Triangles;

	return true;
}

DecalCreater::DecalCreater() : decalInfos_(SUEDE_MAX_DECALS) {
	material_ = NewMaterial();
	material_->SetShader(Resources::instance()->FindShader("builtin/decal"));
	material_->SetRenderQueue((int)RenderQueue::Overlay - 500);
}

void DecalCreater::GetDecals(std::vector<Decal>& container) {
	container.reserve(decalInfos_.size());

	for (DecalInfo* info : decalInfos_) {
		container.push_back(info->decal);
	}
}

bool DecalCreater::CreateProjectorDecal(Camera camera, Projector p, std::vector<Entity>& entities, Plane planes[6]) {
	for (std::vector<Entity>::iterator ite = entities.begin(); ite != entities.end(); ++ite) {
		Entity entity = *ite;
		if (entity == p) { continue; }

		DecalInfo* info = decalInfos_.spawn();
		if (info == nullptr) {
			Debug::LogError("too many decals");
			return false;
		}

		if (!CreateEntityDecal(camera, *info, entity, planes)) {
			decalInfos_.recycle(info);
			continue;
		}

		info->texture = p->GetTexture();
		info->matrix = p->GetProjectionMatrix() * p->GetTransform()->GetWorldToLocalMatrix();

		CreateDecal(info);
	}

	return true;
}

void DecalCreater::CreateDecal(DecalInfo* info) {
	glm::mat4 biasMatrix = glm::scale(glm::translate(glm::mat4(1), glm::vec3(0.5f)), glm::vec3(0.5f));
	Material decalMaterial = suede_dynamic_cast<Material>(material_->Clone());

	decalMaterial->SetMatrix4(Variables::DecalMatrix, biasMatrix * info->matrix);
	decalMaterial->SetTexture(Variables::MainTexture, info->texture);

	Mesh mesh = NewMesh();

	MeshAttribute attribute;
	attribute.topology = info->topology;
	attribute.indexes = info->indexes;
	attribute.positions = info->positions;

	mesh->SetAttribute(attribute);

	SubMesh subMesh = NewSubMesh();
	TriangleBias bias{ info->indexes.size() };
	subMesh->SetTriangleBias(bias);

	mesh->AddSubMesh(subMesh);

	info->decal.mesh = mesh;
	info->decal.material = decalMaterial;
}

bool DecalCreater::ClampMesh(Camera camera, std::vector<glm::vec3>& triangles, Entity entity, Plane planes[6]) {
	Mesh mesh = SUEDE_GET_COMPONENT(entity, MeshFilter)->GetMesh();
	glm::vec3 cameraPosition = entity->GetTransform()->InverseTransformPoint(camera->GetTransform()->GetPosition());

	uint* indexes = mesh->MapIndexes();
	glm::vec3* vertices = mesh->MapVertices();

	for (SubMesh subMesh : mesh->GetSubMeshes()) {
		const TriangleBias& bias = subMesh->GetTriangleBias();

		// SUEDE TODO: use triangle strip?
		for (int j = 0; j < bias.indexCount; j += 3) {
			std::vector<glm::vec3> polygon;
			uint index0 = indexes[bias.baseIndex + j] + bias.baseVertex;
			uint index1 = indexes[bias.baseIndex + j + 1] + bias.baseVertex;
			uint index2 = indexes[bias.baseIndex + j + 2] + bias.baseVertex;

			glm::vec3 vs[] = { vertices[index0], vertices[index1], vertices[index2] };

			if (!GeometryUtility::IsFrontFace(vs, cameraPosition)) {
				continue;
			}

			vs[0] = entity->GetTransform()->TransformPoint(vs[0]);
			vs[1] = entity->GetTransform()->TransformPoint(vs[1]);
			vs[2] = entity->GetTransform()->TransformPoint(vs[2]);

			GeometryUtility::ClampTriangle(polygon, vs, planes, 6);
			GeometryUtility::Triangulate(triangles, polygon, glm::cross(vs[1] - vs[0], vs[2] - vs[1]));
		}
	}

	mesh->UnmapIndexes();
	mesh->UnmapVertices();

	return triangles.size() >= 3;
}
