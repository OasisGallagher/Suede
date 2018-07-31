#include <algorithm>

#include "pipeline.h"
#include "profiler.h"
#include "statistics.h"
#include "api/glutils.h"
#include "matrixbuffer.h"
#include "internal/base/vertexattrib.h"
#include "internal/base/renderdefines.h"

template <class T>
inline int __compare(T lhs, T rhs) {
	if (lhs == rhs) { return 0; }
	return lhs < rhs ? -1 : 1;
}

typedef bool(*RenderableComparer)(const Renderable& lhs, const Renderable& rhs);

#define COMPARE(lhs, rhs)	{ int n = __compare(lhs, rhs); if (n != 0) { return n; } }

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
	: renderables_(INIT_RENDERABLE_CAPACITY), matrices_(INIT_RENDERABLE_CAPACITY * 2), timeQuerier_(0)
	, nrenderables_(0) , oldPass_(-1), ndrawcalls_(0), ntriangles_(0), nmeshChanges_(0), nmaterialChanges_(0) {
	switch_state = Profiler::get()->CreateSample();
	update_ubo = Profiler::get()->CreateSample();
	update_tbo = Profiler::get()->CreateSample();
	draw_call = Profiler::get()->CreateSample();
	update_offset = Profiler::get()->CreateSample();
	update_matrices = Profiler::get()->CreateSample();
	gather_instances = Profiler::get()->CreateSample();
	update_pipeline = Profiler::get()->CreateSample();
	stat_and_output = Profiler::get()->CreateSample();
}

Pipeline::~Pipeline() {
 	Profiler::get()->ReleaseSample(switch_state);
 	Profiler::get()->ReleaseSample(update_ubo);
	Profiler::get()->ReleaseSample(update_tbo);
	Profiler::get()->ReleaseSample(draw_call);
	Profiler::get()->ReleaseSample(update_offset);
	Profiler::get()->ReleaseSample(update_matrices);
 	Profiler::get()->ReleaseSample(gather_instances);
 	Profiler::get()->ReleaseSample(update_pipeline);
	Profiler::get()->ReleaseSample(stat_and_output);
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

void Pipeline::Run(bool __tmpIsRendering) {
	if (__tmpIsRendering) {
		timeQuerier_ = GpuQuerier::get()->Start(QueryTypeTimeElapsed, this);
	}

	update_pipeline->Restart();

	update_tbo->Restart();
	MatrixBuffer::get()->Update(nrenderables_, &matrices_[0]);
	update_tbo->Stop();

	targetTexture_->BindWrite(normalizedRect_);

	uint from = 0;
	draw_call->Reset();
	switch_state->Reset();
	update_offset->Reset();

	for (std::vector<uint>::iterator ite = ranges_.begin(); ite != ranges_.end(); ++ite) {
		Renderable& first = renderables_[from];
		if (first.material->IsPassEnabled(first.pass)) {
			if (renderables_[from].instance != 0) {
				Render(renderables_[from], renderables_[from].instance, 0);
			}
			else {
				RenderInstances(from, *ite);
			}
		}

		from = *ite;
	}

	stat_and_output->Restart();
	Statistics::get()->AddTriangles(ntriangles_);
	Statistics::get()->AddDrawcalls(ndrawcalls_);

	if (__tmpIsRendering) {
		debugDumpPipelineAndRanges(ranges_);
	}

	Debug::Output("[Pipeline::Update::nrenderables]\t%d", nrenderables_);
	Debug::Output("[Pipeline::Update::ndrawcalls]\t%d", ndrawcalls_);
	Debug::Output("[Pipeline::Update::nmeshChanges]\t%d", nmeshChanges_);
	Debug::Output("[Pipeline::Update::nmaterialChanges]\t%d", nmaterialChanges_);
	Debug::Output("[Pipeline::Update::update_matrices]\t%.5f", update_matrices->GetElapsedSeconds());
	Debug::Output("[Pipeline::Update::update_ubo]\t%.5f", update_ubo->GetElapsedSeconds());
	Debug::Output("[Pipeline::Update::update_tbo]\t%.5f", update_tbo->GetElapsedSeconds());
	Debug::Output("[Pipeline::Update::update_offset]\t%.5f", update_offset->GetElapsedSeconds());
	Debug::Output("[Pipeline::Update::draw_call]\t%.5f", draw_call->GetElapsedSeconds());
	Debug::Output("[Pipeline::Update::switch_state]\t%.5f", switch_state->GetElapsedSeconds());
	stat_and_output->Stop();

	ResetState();
	targetTexture_->Unbind();

	update_pipeline->Stop();
	Debug::Output("[Pipeline::Update::stat_and_output]\t%.5f", stat_and_output->GetElapsedSeconds());
	Debug::Output("[Pipeline::Update::update_pipeline]\t%.5f", update_pipeline->GetElapsedSeconds());

	if (__tmpIsRendering) {
		GpuQuerier::get()->Stop();
		uint time = GpuQuerier::get()->Wait(timeQuerier_);
		double seconds = time * 10e-9;
	}
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
	Render(renderables_[first], last - first, first * 8);
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

void Pipeline::OnQuerierResult(uint id, uint result) {
	if (id == timeQuerier_) {
		double seconds = result * 1e-9;
	}
}

void Pipeline::Render(Renderable& renderable, uint instance, uint matrixOffset) {
	switch_state->Start();
	UpdateState(renderable);
	switch_state->Stop();

	update_offset->Start();
	GL::VertexAttribI1i(VertexAttribMatrixOffset, matrixOffset);
	update_offset->Stop();

	const TriangleBias& bias = renderable.mesh->GetSubMesh(renderable.subMeshIndex)->GetTriangleBias();

	draw_call->Start();
	GLUtils::DrawElementsInstancedBaseVertex(renderable.mesh->GetTopology(), bias, instance);
	draw_call->Stop();

	++ndrawcalls_;
	ntriangles_ += bias.indexCount / 3;
}

void Pipeline::UpdateState(Renderable& renderable) {
	if (renderable.material != oldMaterial_) {
		if (oldMaterial_) {
			oldMaterial_->Unbind();
		}

		oldPass_ = renderable.pass;
		oldMaterial_ = renderable.material;

		renderable.material->Bind(renderable.pass);
		++nmaterialChanges_;
	}
	else if (oldPass_ != renderable.pass) {
		oldMaterial_->Unbind();
		oldPass_ = renderable.pass;

		renderable.material->Bind(renderable.pass);

		++nmaterialChanges_;
	}

	if (!oldMesh_ || renderable.mesh->GetNativePointer() != oldMesh_->GetNativePointer()) {
		if (oldMesh_) {
			oldMesh_->Unbind();
		}

		oldMesh_ = renderable.mesh;
		renderable.mesh->Bind();

		++nmeshChanges_;
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
	nmeshChanges_ = 0;
	nmaterialChanges_ = 0;

	switch_state->Reset();

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
