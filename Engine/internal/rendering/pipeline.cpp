#include <algorithm>

#include "time2.h"
#include "engine.h"
#include "pipeline.h"
#include "statistics.h"
#include "tools/math2.h"
#include "api/glutils.h"
#include "matrixbuffer.h"
#include "debug/profiler.h"
#include "uniformbuffermanager.h"

#define RENDERABLE_CAPACITY	1024

template <class T>
inline int __compare(T lhs, T rhs) {
	if (lhs == rhs) { return 0; }
	return lhs < rhs ? -1 : 1;
}

typedef bool(*RenderableComparer)(const Renderable& lhs, const Renderable& rhs);

#define COMPARE(lhs, rhs)	if (true) { int n = __compare(lhs, rhs); if (n != 0) { return n; } } else (void)0

static int MeshPredicate(const Renderable& lhs, const Renderable &rhs) {
	COMPARE(lhs.mesh->GetNativePointer(), rhs.mesh->GetNativePointer());
	COMPARE(lhs.subMeshIndex, rhs.subMeshIndex);

	const TriangleBias& bias = lhs.mesh->GetSubMesh(lhs.subMeshIndex)->GetTriangleBias();
	const TriangleBias& otherBias = rhs.mesh->GetSubMesh(rhs.subMeshIndex)->GetTriangleBias();
	COMPARE(bias.indexCount, otherBias.indexCount);
	COMPARE(bias.baseIndex, otherBias.baseIndex);
	COMPARE(bias.baseVertex, otherBias.baseVertex);

	return 0;
}

static int MaterialPredicate(const Renderable& lhs, const Renderable& rhs) {
	const Material& lm = lhs.material, &rm = rhs.material;

	COMPARE(lm->GetRenderQueue(), rm->GetRenderQueue());
	COMPARE(lm, rm);
	COMPARE(lhs.pass, rhs.pass);
	COMPARE(lm->GetPassNativePointer(lhs.pass), rm->GetPassNativePointer(rhs.pass));

	return 0;
}

Pipeline::Pipeline() 
	: renderables_(RENDERABLE_CAPACITY), matrices_(RENDERABLE_CAPACITY * 2)
	, nrenderables_(0) , oldPass_(-1), ndrawcalls_(0), ntriangles_(0) {
	switch_material = Profiler::CreateSample();
	switch_mesh = Profiler::CreateSample();
	update_ubo = Profiler::CreateSample();
	update_tbo = Profiler::CreateSample();
	update_matrices = Profiler::CreateSample();
	gather_instances = Profiler::CreateSample();
	update_pipeline = Profiler::CreateSample();
	rendering = Profiler::CreateSample();
}

Pipeline::~Pipeline() {
 	Profiler::ReleaseSample(switch_material);
 	Profiler::ReleaseSample(switch_mesh);
 	Profiler::ReleaseSample(update_ubo);
	Profiler::ReleaseSample(update_tbo);
	Profiler::ReleaseSample(update_matrices);
 	Profiler::ReleaseSample(gather_instances);
 	Profiler::ReleaseSample(update_pipeline);
 	Profiler::ReleaseSample(rendering);
}

static bool MeshComparer(const Renderable& lhs, const Renderable& rhs) {
	return MeshPredicate(lhs, rhs) < 0;
}

static bool MaterialComparer(const Renderable& lhs, const Renderable& rhs) {
	return MaterialPredicate(lhs, rhs) < 0;
}

static bool MeshMaterialComparer(const Renderable& lhs, const Renderable& rhs) {
	int n = MaterialPredicate(lhs, rhs);
	return n != 0 ? n < 0 : MeshPredicate(lhs, rhs) < 0;
}

static RenderableComparer comparers_[] = {
	MeshComparer, MaterialComparer, MeshMaterialComparer,
};

void Pipeline::Sort(SortMode mode, const glm::mat4& worldToClipMatrix) {
	std::sort(renderables_.begin(), renderables_.begin() + nrenderables_, comparers_[mode]);

	for (int i = 0; i < nrenderables_; ++i) {
		Renderable& renderable = renderables_[i];
		matrices_[i * 2] = renderable.localToWorldMatrix;
		matrices_[i * 2 + 1] = worldToClipMatrix * renderable.localToWorldMatrix;
	}

	GatherInstances(ranges_);
}

void Pipeline::Run() {
	update_tbo->Restart();
	uint size = nrenderables_ * 2 * sizeof(glm::mat4);

	MatrixBuffer::Update(size, &matrices_[0]);

	update_tbo->Stop();

	targetTexture_->BindWrite(normalizedRect_);

 	update_pipeline->Restart();

	rendering->Restart();
	uint from = 0;

	for (std::vector<uint>::iterator ite = ranges_.begin(); ite != ranges_.end(); ++ite) {
		Renderable& first = renderables_[from];
		if (first.material->IsPassEnabled(first.pass)) {
			if (renderables_[from].instance != 0) {
				Render(renderables_[from], renderables_[from].instance);
			}
			else {
				RenderInstances(from, *ite);
			}
		}

		from = *ite;
	}

	Statistics::AddTriangles(ntriangles_);
	Statistics::AddDrawcalls(ndrawcalls_);

	//debugDumpPipelineAndRanges(ranges_);

	rendering->Stop();

	Debug::Output("[Pipeline::Update::nrenderables]\t%d", nrenderables_);
	Debug::Output("[Pipeline::Update::ndrawcalls]\t%d", ndrawcalls_);
	Debug::Output("[Pipeline::Update::update_matrices]\t%.2f", update_matrices->GetElapsedSeconds());
	Debug::Output("[Pipeline::Update::update_ubo]\t%.5f", update_ubo->GetElapsedSeconds());
	Debug::Output("[Pipeline::Update::update_tbo]\t%.5f", update_tbo->GetElapsedSeconds());
	Debug::Output("[Pipeline::Update::rendering]\t%.2f", rendering->GetElapsedSeconds());

	//Debug::Output("[Pipeline::Update::switch_framebuffer]\t%.2f", switch_framebuffer->GetElapsedSeconds());
	//Debug::Output("[Pipeline::Update::switch_material]\t%.2f", switch_material->GetElapsedSeconds());
	//Debug::Output("[Pipeline::Update::switch_mesh]\t%.2f", switch_mesh->GetElapsedSeconds());

	update_pipeline->Stop();
	/*Debug::Output("[Pipeline::Update::update_pipeline]\t%.2f", update_pipeline->GetElapsedSeconds());*/

	ResetState();
	targetTexture_->Unbind();
}

void Pipeline::GatherInstances(std::vector<uint>& ranges) {
	uint base = 0;
	for (uint i = 0; i < nrenderables_; ++i) {
		// particle system.
		if (renderables_[i].instance != 0) {
			// render instanced renderables.
			if (i > base) {
				ranges.push_back(i);
			}

			ranges.push_back(i + 1);
			base = i + 1;
		}
		else if (i != base && !renderables_[base].IsInstance(renderables_[i])) {
			ranges.push_back(i);
			base = i;
		}
	}

	if (base != nrenderables_) {
		ranges.push_back(nrenderables_);
	}
}

void Pipeline::RenderInstances(uint first, uint last) {
	Renderable& renderable = renderables_[first];
	static int maxInstances = GLUtils::GetLimits(GLLimitsMaxUniformBlockSize) / sizeof(EntityMatricesUniforms);
	int instanceCount = last - first;

	static SharedMatrixBufferOffsetUniformBuffer p;
	p.matrixBufferOffset = first * 8;
	UniformBufferManager::UpdateSharedBuffer(SharedMatrixBufferOffsetUniformBuffer::GetName(), &p, 0, sizeof(p));

	for (int i = 0; i < instanceCount; ) {
		int count = Math::Min(instanceCount - i, maxInstances);
		update_ubo->Start();
		UniformBufferManager::UpdateSharedBuffer(EntityMatricesUniforms::GetName(), &matrices_[(i + first) * 2], 0, sizeof(glm::mat4) * (count * 2));
		update_ubo->Stop();

		Render(renderable, last - first);
		i += count;
	}
}

void Pipeline::AddRenderable(Mesh mesh, uint subMeshIndex, Material material, uint pass, const glm::mat4& localToWorldMatrix, uint instance) {
	if (nrenderables_ == renderables_.size()) {
		matrices_.resize(4 * nrenderables_);
		renderables_.resize(2 * nrenderables_);
	}

	Renderable& renderable = renderables_[nrenderables_++];
	renderable.instance = instance;
	renderable.mesh = mesh;
	renderable.subMeshIndex = subMeshIndex;
	renderable.material = material;
	renderable.pass = pass;
	renderable.localToWorldMatrix = localToWorldMatrix;

	if (mesh->GetTopology() < 0 || mesh->GetTopology() > 3) {
		Debug::Break();
	}
}

void Pipeline::AddRenderable(Mesh mesh, Material material, uint pass, const glm::mat4& localToWorldMatrix, uint instance /*= 0 */) {
	for (int i = 0; i < mesh->GetSubMeshCount(); ++i) {
		AddRenderable(mesh, i, material, 0, localToWorldMatrix, instance);
	}
}

void Pipeline::Render(Renderable& renderable, uint instance) {
	UpdateState(renderable);

	const TriangleBias& bias = renderable.mesh->GetSubMesh(renderable.subMeshIndex)->GetTriangleBias();

	//if (renderable.instance == 0) {
	//	Debug::Break();
		// TODO: update c_localToWorldMatrix, c_localToClipMatrix to material.
		//GLUtil::DrawElementsBaseVertex(renderable.mesh->GetTopology(), bias);
	//}
	//else {
	GLUtils::DrawElementsInstancedBaseVertex(renderable.mesh->GetTopology(), bias, instance);
	//}

	++ndrawcalls_;
	ntriangles_ += bias.indexCount / 3;
}

void Pipeline::UpdateState(Renderable& renderable) {
	if (renderable.material != oldMaterial_) {
		switch_material->Start();
		if (oldMaterial_) {
			oldMaterial_->Unbind();
		}

		oldPass_ = renderable.pass;
		oldMaterial_ = renderable.material;

		renderable.material->Bind(renderable.pass);
		switch_material->Stop();
	}
	else if (oldPass_ != renderable.pass) {
		renderable.material->Bind(renderable.pass);
		oldPass_ = renderable.pass;
	}

	if (!oldMesh_ || renderable.mesh->GetNativePointer() != oldMesh_->GetNativePointer()) {
		switch_mesh->Start();
		
		if (oldMesh_) {
			oldMesh_->Unbind();
		}

		oldMesh_ = renderable.mesh;

		renderable.mesh->Bind();
		switch_mesh->Stop();
	}
}

void Pipeline::Clear() {
	ranges_.clear();

	for (uint i = 0; i < nrenderables_; ++i) {
		renderables_[i].Clear();
	}

	nrenderables_ = 0;
}

RenderTexture Pipeline::GetTargetTexture() {
	return targetTexture_;
}

void Pipeline::SetTargetTexture(RenderTexture value, const Rect& normalizedRect) {
	targetTexture_ = value;
	normalizedRect_ = normalizedRect;
}

Pipeline& Pipeline::operator=(const Pipeline& other) {
	ranges_ = other.ranges_;
	matrices_ = other.matrices_;

	renderables_ = other.renderables_;
	nrenderables_ = other.nrenderables_;
	return *this;
}

void Pipeline::ResetState() {
	ndrawcalls_ = 0;
	ntriangles_ = 0;

	switch_mesh->Reset();
	switch_material->Reset();

	update_ubo->Reset();
	update_matrices->Reset();

	if (oldMaterial_) {
		oldMaterial_->Unbind();
		oldMaterial_.reset();
	}

	if (oldMesh_) {
		oldMesh_->Unbind();
		oldMesh_.reset();
	}

	oldPass_ = 0;
}

bool Renderable::IsInstance(const Renderable& other) const {
	return IsMeshInstanced(other) && IsMaterialInstanced(other);
}

void Renderable::Clear() {
	instance = 0;
	mesh.reset();
	material.reset();
}

bool Renderable::IsMeshInstanced(const Renderable& other) const {
	if (mesh->GetNativePointer() != other.mesh->GetNativePointer()
		|| subMeshIndex != other.subMeshIndex) {
		return false;
	}

	const TriangleBias& bias = mesh->GetSubMesh(subMeshIndex)->GetTriangleBias();
	const TriangleBias& otherBias = other.mesh->GetSubMesh(subMeshIndex)->GetTriangleBias();
	return bias.indexCount == otherBias.indexCount
		&& bias.baseVertex == otherBias.baseVertex && bias.baseIndex == otherBias.baseIndex;
}

bool Renderable::IsMaterialInstanced(const Renderable& other) const {
	return material == other.material && pass == other.pass;
}
