#include "rendererinternal.h"

#include <algorithm>

#include "geometryutility.h"
#include "builtinproperties.h"
#include "internal/base/renderdefines.h"
#include "internal/base/materialinternal.h"

#include "internal/rendering/pipeline.h"

Renderer::Renderer(void* d) : Component(d) {}
void Renderer::AddSharedMaterial(Material* material) { _suede_dptr()->AddSharedMaterial(material); }
Material* Renderer::GetMaterial(uint index) { return _suede_dptr()->GetMaterial(index); }
Material* Renderer::GetSharedMaterial(uint index) { return _suede_dptr()->GetSharedMaterial(index); }
bool Renderer::IsMaterialInstantiated(uint index) { return _suede_dptr()->IsMaterialInstantiated(index); }
void Renderer::SetMaterial(uint index, Material* value) { _suede_dptr()->SetMaterial(index, value); }
void Renderer::RemoveMaterialAt(uint index) { _suede_dptr()->RemoveMaterialAt(index); }
uint Renderer::GetMaterialCount() { return _suede_dptr()->GetMaterialCount(); }
void Renderer::UpdateMaterialProperties() { _suede_dptr()->UpdateMaterialProperties(); }
const Bounds& Renderer::GetBounds() { return  _suede_dptr()->GetBounds(); }

MeshRenderer::MeshRenderer() : Renderer(new MeshRendererInternal) {}

ParticleRenderer::ParticleRenderer() : Renderer(new ParticleRendererInternal) {}

SkinnedMeshRenderer::SkinnedMeshRenderer() : Renderer(new SkinnedMeshRendererInternal) {}
void SkinnedMeshRenderer::SetSkeleton(Skeleton* value) { _suede_dptr()->SetSkeleton(value); }

SUEDE_DEFINE_COMPONENT_INTERNAL(Renderer, Component)
SUEDE_DEFINE_COMPONENT_INTERNAL(MeshRenderer, Renderer)
SUEDE_DEFINE_COMPONENT_INTERNAL(ParticleRenderer, Renderer)
SUEDE_DEFINE_COMPONENT_INTERNAL(SkinnedMeshRenderer, Renderer)

RendererInternal::RendererInternal(ObjectType type) : ComponentInternal(type) {
}

Material* RendererInternal::GetMaterial(uint index) {
	auto& p = materials_[index];
	if (!p.material) { p.material = p.sharedMaterial->Clone(); }
	return p.material.get();
}

void RendererInternal::RemoveMaterialAt(uint index) {
	SUEDE_ASSERT(index < materials_.size());
	materials_.erase(materials_.begin() + index);
}

const Bounds& RendererInternal::GetBounds() {
	UpdateBounds();
	return bounds_;
}

void SkinnedMeshRendererInternal::UpdateMaterialProperties() {
	for (int i = 0; i < GetMaterialCount(); ++i) {
		Material* material = GetMaterial(i);
		material->SetMatrix4Array(BuiltinProperties::BoneToRootMatrices, skeleton_->GetBoneToRootMatrices(), MAX_BONE_COUNT);
	}
}

void SkinnedMeshRendererInternal::UpdateBounds() {
	bounds_.Clear();
	
	std::vector<Vector3> points;
	Vector3 min(std::numeric_limits<float>::max()), max(std::numeric_limits<float>::lowest());

	Bounds boneBounds;
	Matrix4* matrices = skeleton_->GetBoneToRootMatrices();

	for (uint i = 0; i < skeleton_->GetBoneCount(); ++i) {
		SkeletonBone* bone = skeleton_->GetBone(i);
		GeometryUtility::GetCuboidCoordinates(points, bone->bounds.center, bone->bounds.size);
		for (uint j = 0; j < points.size(); ++j) {
			Vector4 p = matrices[i] * Vector4(points[j].x, points[j].y, points[j].z, 1);
			points[j] = GetTransform()->TransformPoint(Vector3(p.x, p.y, p.z));

			min = Vector3::Min(min, points[j]);
			max = Vector3::Max(max, points[j]);
		}

		boneBounds.SetMinMax(min, max);
		bounds_.Encapsulate(boneBounds);
	}
}

ParticleRendererInternal::ParticleRendererInternal()
	: RendererInternal(ObjectType::ParticleRenderer) {
}

void ParticleRendererInternal::UpdateBounds() {
	ParticleSystem* particleSystem = gameObject_->GetComponent<ParticleSystem>();
	if (particleSystem != nullptr) {
		bounds_ = particleSystem->GetMaxBounds();
	}
}

void MeshRendererInternal::UpdateBounds() {
	MeshProvider* meshProvider = gameObject_->GetComponent<MeshProvider>();
	Mesh* mesh = (meshProvider != nullptr) ? meshProvider->GetMesh() : nullptr;
	if (mesh != nullptr) {
		bounds_ = mesh->GetBounds();

		std::vector<Vector3> points;
		GeometryUtility::GetCuboidCoordinates(points, bounds_.center, bounds_.size);

		Vector3 min(std::numeric_limits<float>::max()), max(std::numeric_limits<float>::lowest());
		for (Vector3& p : points) {
			p = GetTransform()->TransformPoint(p);
			min = Vector3::Min(p, min);
			max = Vector3::Max(p, max);
		}

		bounds_.SetMinMax(min, max);
	}
}
