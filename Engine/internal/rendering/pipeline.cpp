#include <algorithm>

#include "time2.h"
#include "engine.h"
#include "pipeline.h"
#include "statistics.h"
#include "tools/math2.h"
#include "api/gllimits.h"
#include "debug/profiler.h"
#include "uniformbuffermanager.h"

template <class T>
inline int __compare(T lhs, T rhs) {
	if (lhs == rhs) { return 0; }
	return lhs < rhs ? -1 : 1;
}

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
	COMPARE(lhs.target, rhs.target);
	COMPARE(lm, rm);
	COMPARE(lhs.pass, rhs.pass);
	COMPARE(lm->GetPassNativePointer(lhs.pass), rm->GetPassNativePointer(rhs.pass));

	return 0;
}

Pipeline::Pipeline() :renderables_(1024), nrenderables_(0)
	, oldPass_(-1), ndrawcalls_(0), ntriangles_(0) {
	switch_material = Profiler::CreateSample();
	switch_framebuffer = Profiler::CreateSample();
	switch_mesh = Profiler::CreateSample();
	update_ubo = Profiler::CreateSample();
	gather_instances = Profiler::CreateSample();
	update_pipeline = Profiler::CreateSample();
	rendering = Profiler::CreateSample();

	Engine::AddFrameEventListener(this);
}

Pipeline::~Pipeline() {
	Profiler::ReleaseSample(switch_material);
	Profiler::ReleaseSample(switch_framebuffer);
	Profiler::ReleaseSample(switch_mesh);
	Profiler::ReleaseSample(update_ubo);
	Profiler::ReleaseSample(gather_instances);
	Profiler::ReleaseSample(update_pipeline);
	Profiler::ReleaseSample(rendering);

	Engine::RemoveFrameEventListener(this);
}

void Pipeline::OnFrameEnter() {

}

void Pipeline::OnFrameLeave() {

}

void Pipeline::Sort(SortMode mode) {
	struct MeshComparer {
		bool operator ()(const Renderable& lhs, const Renderable& rhs) const {
			return MeshPredicate(lhs, rhs) < 0;
		}
	};

	struct MaterialComparer {
		bool operator ()(const Renderable& lhs, const Renderable& rhs) const {
			return MaterialPredicate(lhs, rhs) < 0;
		}
	};

	struct MeshMaterialComparer {
		bool operator ()(const Renderable& lhs, const Renderable& rhs) const {
			int n = MaterialPredicate(lhs, rhs);
			return n != 0 ? n < 0 : MeshPredicate(lhs, rhs) < 0;
		}
	};

	switch (mode) {
		case SortModeMesh:
			std::sort(renderables_.begin(), renderables_.begin() + nrenderables_, MeshComparer());
			break;
		case SortModeMaterial:
			std::sort(renderables_.begin(), renderables_.begin() + nrenderables_, MaterialComparer());
			break;
		case SortModeMeshMaterial:
			std::sort(renderables_.begin(), renderables_.begin() + nrenderables_, MeshMaterialComparer());
			break;
	}
}

void Pipeline::Run(const glm::mat4& worldToClipMatrix) {
	update_pipeline->Restart();

	gather_instances->Restart();
	std::vector<uint> ranges;
	GatherInstances(ranges);
	gather_instances->Stop();
	Debug::Output("[Pipeline::Update::gather]\t%.2f", gather_instances->GetElapsedSeconds());

	rendering->Restart();
	uint from = 0;

	for (std::vector<uint>::iterator ite = ranges.begin(); ite != ranges.end(); ++ite) {
		if (renderables_[from].instance != 0) {
			Render(renderables_[from]);
		}
		else {
			RenderInstances(from, *ite, worldToClipMatrix);
		}

		from = *ite;
	}

	Statistics::AddTriangles(ntriangles_);
	Statistics::AddDrawcalls(ndrawcalls_);

	debugDumpPipelineAndRanges(ranges);

	rendering->Stop();

	Debug::Output("[Pipeline::Update::nrenderables]\t%d", nrenderables_);
	Debug::Output("[Pipeline::Update::ndrawcalls]\t%d", ndrawcalls_);
	Debug::Output("[Pipeline::Update::update_ubo]\t%.2f", update_ubo->GetElapsedSeconds());
	Debug::Output("[Pipeline::Update::rendering]\t%.2f", rendering->GetElapsedSeconds());

	Debug::Output("[Pipeline::Update::switch_framebuffer]\t%.2f", switch_framebuffer->GetElapsedSeconds());
	Debug::Output("[Pipeline::Update::switch_material]\t%.2f", switch_material->GetElapsedSeconds());
	Debug::Output("[Pipeline::Update::switch_mesh]\t%.2f", switch_mesh->GetElapsedSeconds());

	update_pipeline->Stop();
	Debug::Output("[Pipeline::Update::update_pipeline]\t%.2f", update_pipeline->GetElapsedSeconds());

	Clear();
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

void Pipeline::RenderInstances(uint first, uint last, const glm::mat4& worldToClipMatrix) {
	Renderable& renderable = renderables_[first];
	static int maxInstances = GLLimits::Get(GLLimitsMaxUniformBlockSize) / sizeof(EntityMatricesUniforms);
	int instanceCount = last - first;

	for (int i = 0; i < instanceCount; ) {
		int count = Math::Min(instanceCount - i, maxInstances);
		renderable.instance = count;

		std::vector<glm::mat4> matrices;
		matrices.reserve(2 * count);
		for (int j = i + first, max = j + count; j < max; ++j) {
			matrices.push_back(renderables_[j].localToWorldMatrix);
			matrices.push_back(worldToClipMatrix * renderables_[j].localToWorldMatrix);
		}

		update_ubo->Restart();
		UniformBufferManager::UpdateSharedBuffer(EntityMatricesUniforms::GetName(), &matrices[0], 0, sizeof(glm::mat4) * matrices.size());
		update_ubo->Stop();

		Render(renderable);
		i += count;
	}
}

#include <fstream>
void Pipeline::debugDumpPipelineAndRanges(std::vector<uint>& ranges) {
	static bool dumped = false;
	if (nrenderables_ > 50 && !dumped) {
		std::ofstream ofs("pipeline_dump.txt");
		ofs << "Index\tQueue\tMaterial\tPass\tShader\tMesh\tSubMesh\tIndexCount\tBaseIndex\tBaseVertex\n";
		uint j = 0;
		for (uint i = 0; i < nrenderables_; ++i) {
			Renderable& r = renderables_[i];
			ofs << ((i + 1 == ranges[j]) ? std::to_string(ranges[j]) : "")
				<< "\t" << r.material->GetRenderQueue()
				<< "\t" << r.material.get()
				<< "\t" << r.pass
				<< "\t" << r.material->GetPassNativePointer(r.pass)
				<< "\t" << r.mesh->GetNativePointer()
				<< "\t" << r.subMeshIndex
				<< "\t" << r.mesh->GetSubMesh(r.subMeshIndex)->GetTriangleBias().indexCount
				<< "\t" << r.mesh->GetSubMesh(r.subMeshIndex)->GetTriangleBias().baseIndex
				<< "\t" << r.mesh->GetSubMesh(r.subMeshIndex)->GetTriangleBias().baseVertex
				<< std::endl;

			if (i + 1 == ranges[j]) { ++j; }
		}

		ofs.close();

		dumped = true;
	}
}

void Pipeline::AddRenderable(Mesh mesh, uint subMeshIndex, Material material, uint pass, RenderTexture target, const Rect& normalizedRect, const glm::mat4& localToWorldMatrix, uint instance) {
	if (nrenderables_ == renderables_.size()) {
		renderables_.resize(2 * nrenderables_);
	}

	Renderable& renderable = renderables_[nrenderables_++];
	renderable.instance = instance;
	renderable.mesh = mesh;
	renderable.subMeshIndex = subMeshIndex;
	renderable.material = material;
	renderable.pass = pass;
	renderable.target = target;
	renderable.normalizedRect = normalizedRect;
	renderable.localToWorldMatrix = localToWorldMatrix;
}

void Pipeline::AddRenderable(Mesh mesh, Material material, uint pass, RenderTexture target, const Rect& normalizedRect, const glm::mat4& localToWorldMatrix, uint instance /*= 0 */) {
	for (int i = 0; i < mesh->GetSubMeshCount(); ++i) {
		AddRenderable(mesh, i, material, 0, target, normalizedRect, localToWorldMatrix);
	}
}

void Pipeline::Render(Renderable& renderable) {
	UpdateState(renderable);

	const TriangleBias& bias = renderable.mesh->GetSubMesh(renderable.subMeshIndex)->GetTriangleBias();

	if (renderable.instance == 0) {
		Debug::Break();
		// TODO: update c_localToWorldMatrix, c_localToClipMatrix to material.
		//GLUtil::DrawElementsBaseVertex(renderable.mesh->GetTopology(), bias);
	}
	else {
		GLUtil::DrawElementsInstancedBaseVertex(renderable.mesh->GetTopology(), bias, renderable.instance);
	}

	++ndrawcalls_;
	ntriangles_ += bias.indexCount / 3;
}

void Pipeline::UpdateState(Renderable& renderable) {
	if (!oldTarget_ || oldTarget_ != renderable.target) {
		switch_framebuffer->Start();
		if (oldTarget_) {
			oldTarget_->Unbind();
		}

		oldTarget_ = renderable.target;
		renderable.target->BindWrite(renderable.normalizedRect);

		switch_framebuffer->Stop();
	}

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
	ndrawcalls_ = 0;
	ntriangles_ = 0;

	ResetState();

	switch_mesh->Clear();
	switch_material->Clear();
	switch_framebuffer->Clear();

	for (uint i = 0; i < nrenderables_; ++i) {
		renderables_[i].Clear();
	}

	nrenderables_ = 0;
}

void Pipeline::ResetState() {
	if (oldTarget_) {
		oldTarget_->Unbind();
		oldTarget_.reset();
	}

	if (oldMaterial_) {
		oldMaterial_->Unbind();
		oldMaterial_.reset();
	}

	if (oldMesh_) {
		oldMesh_->Unbind();
		oldMesh_.reset();
	}
}

bool Renderable::IsInstance(const Renderable& other) const {
	return IsFramebufferInstanced(other) && IsMeshInstanced(other) && IsMaterialInstanced(other);
}

void Renderable::Clear() {
	instance = 0;
	mesh.reset();
	material.reset();
}

bool Renderable::IsMeshInstanced(const Renderable & other) const {
	if (mesh->GetNativePointer() != other.mesh->GetNativePointer()
		|| subMeshIndex != other.subMeshIndex) {
		return false;
	}

	const TriangleBias& bias = mesh->GetSubMesh(subMeshIndex)->GetTriangleBias();
	const TriangleBias& otherBias = other.mesh->GetSubMesh(subMeshIndex)->GetTriangleBias();
	return bias.indexCount == otherBias.indexCount
		&& bias.baseVertex == otherBias.baseVertex && bias.baseIndex == otherBias.baseIndex;
}

bool Renderable::IsMaterialInstanced(const Renderable & other) const {
	return material == other.material && pass == other.pass;
}

bool Renderable::IsFramebufferInstanced(const Renderable & other) const {
	return target == other.target;
}
