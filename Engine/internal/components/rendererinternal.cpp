#include "rendererinternal.h"

#include <algorithm>

#include "geometryutility.h"
#include "builtinproperties.h"
#include "internal/base/renderdefines.h"
#include "internal/rendering/pipeline.h"

Renderer::Renderer(void* d) : Component(d) {}
void Renderer::AddMaterial(Material* material) { _suede_dptr()->AddMaterial(material); }
Material* Renderer::GetMaterial(uint index) { return _suede_dptr()->GetMaterial(index); }
void Renderer::SetMaterial(uint index, Material* value) { _suede_dptr()->SetMaterial(index, value); }
void Renderer::RemoveMaterial(Material* material) { _suede_dptr()->RemoveMaterial(material); }
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

RendererInternal::~RendererInternal() {
}

void RendererInternal::RemoveMaterial(Material* material) {
	materials_.erase(
		std::remove(materials_.begin(), materials_.end(), material),
		materials_.end()
	);
}

void RendererInternal::RemoveMaterialAt(uint index) {
	SUEDE_ASSERT(index < materials_.size());
	materials_.erase(materials_.begin() + index);
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
	Mesh* mesh = nullptr;
	if (meshProvider != nullptr) { mesh = meshProvider->GetMesh(); }
	if (mesh == nullptr) { return; }

	bounds_ = mesh->GetBounds();
	bounds_.Translate(GetTransform()->GetPosition());
}
