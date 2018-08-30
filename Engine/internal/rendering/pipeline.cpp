#include <algorithm>

#include "pipeline.h"
#include "profiler.h"
#include "statistics.h"
#include "matrixbuffer.h"
#include "../api/glutils.h"
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
	: renderables_(INIT_RENDERABLE_CAPACITY), matrices_(INIT_RENDERABLE_CAPACITY * 2), nrenderables_(0) {
	memset(&counters_, 0, sizeof(counters_));
	oldStates_.Reset();
}

Pipeline::~Pipeline() {
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

void Pipeline::Run() {
	samples_.update_pipeline->Restart();

	samples_.update_tbo->Restart();
	MatrixBuffer::instance()->Update(nrenderables_, &matrices_[0]);
	samples_.update_tbo->Stop();

	targetTexture_->BindWrite(normalizedRect_);

	uint from = 0;
	samples_.draw_call->Reset();
	samples_.switch_state->Reset();
	samples_.update_offset->Reset();

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

	targetTexture_->Unbind();

	samples_.stat_and_output->Restart();
	Statistics::instance()->AddTriangles(counters_.triangles);
	Statistics::instance()->AddDrawcalls(counters_.drawcalls);
	samples_.stat_and_output->Stop();
	
	samples_.update_pipeline->Stop();

	Debug::Output("[Pipeline::Update::renderables]\t%d", nrenderables_);
	Debug::Output("[Pipeline::Update::drawcalls]\t%d", counters_.drawcalls);
	Debug::Output("[Pipeline::Update::meshChanges]\t%d", counters_.meshChanges);
	Debug::Output("[Pipeline::Update::materialChanges]\t%d", counters_.materialChanges);
	Debug::Output("[Pipeline::Update::update_matrices]\t%.5f", samples_.update_matrices->GetElapsedSeconds());
	Debug::Output("[Pipeline::Update::update_ubo]\t%.5f", samples_.update_ubo->GetElapsedSeconds());
	Debug::Output("[Pipeline::Update::update_tbo]\t%.5f", samples_.update_tbo->GetElapsedSeconds());
	Debug::Output("[Pipeline::Update::update_offset]\t%.5f", samples_.update_offset->GetElapsedSeconds());
	Debug::Output("[Pipeline::Update::draw_call]\t%.5f", samples_.draw_call->GetElapsedSeconds());
	Debug::Output("[Pipeline::Update::switch_state]\t%.5f", samples_.switch_state->GetElapsedSeconds());
	Debug::Output("[Pipeline::Update::stat_and_output]\t%.5f", samples_.stat_and_output->GetElapsedSeconds());
	Debug::Output("[Pipeline::Update::update_pipeline]\t%.5f", samples_.update_pipeline->GetElapsedSeconds());

	ResetState();
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
}

void Pipeline::AddRenderable(Mesh mesh, Material material, uint pass, const glm::mat4& localToWorldMatrix, uint instance /*= 0 */) {
	for (int i = 0; i < mesh->GetSubMeshCount(); ++i) {
		AddRenderable(mesh, i, material, 0, localToWorldMatrix, instance);
	}
}

void Pipeline::Render(Renderable& renderable, uint instance, uint matrixOffset) {
	samples_.switch_state->Start();
	UpdateState(renderable);
	samples_.switch_state->Stop();

	samples_.update_offset->Start();
	GL::VertexAttribI1i(VertexAttribMatrixOffset, matrixOffset);
	samples_.update_offset->Stop();

	const TriangleBias& bias = renderable.mesh->GetSubMesh(renderable.subMeshIndex)->GetTriangleBias();

	samples_.draw_call->Start();
	GLUtils::DrawElementsInstancedBaseVertex(renderable.mesh->GetTopology(), bias, instance);
	samples_.draw_call->Stop();

	++counters_.drawcalls;

	MeshTopology topology = renderable.mesh->GetTopology();
	if (topology == MeshTopology::Triangles) {
		counters_.triangles += bias.indexCount / 3;
	}
	else if(topology == MeshTopology::TriangleStripe) {
		counters_.triangles += bias.indexCount - 2;
	}
}

void Pipeline::UpdateState(Renderable& renderable) {
	if (renderable.material != oldStates_.material) {
		if (oldStates_.material) {
			oldStates_.material->Unbind();
		}

		oldStates_.pass = renderable.pass;
		oldStates_.material = renderable.material;

		renderable.material->Bind(renderable.pass);
		++counters_.materialChanges;
	}
	else if (oldStates_.pass != renderable.pass) {
		oldStates_.material->Unbind();
		oldStates_.pass = renderable.pass;

		renderable.material->Bind(renderable.pass);

		++counters_.materialChanges;
	}

	if (!oldStates_.mesh || renderable.mesh->GetNativePointer() != oldStates_.mesh->GetNativePointer()) {
		if (oldStates_.mesh) {
			oldStates_.mesh->Unbind();
		}

		oldStates_.mesh = renderable.mesh;
		renderable.mesh->Bind();

		++counters_.meshChanges;
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
	samples_.Reset();
	oldStates_.Reset();
	memset(&counters_, 0, sizeof(counters_));
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

void Pipeline::States::Reset() {
	if (material) {
		material->Unbind();
		material.reset();
	}

	if (mesh) {
		mesh->Unbind();
		mesh.reset();
	}

	pass = -1;
}

Pipeline::Samples::Samples() {
	switch_state = Profiler::instance()->CreateSample();
	update_ubo = Profiler::instance()->CreateSample();
	update_tbo = Profiler::instance()->CreateSample();
	draw_call = Profiler::instance()->CreateSample();
	update_offset = Profiler::instance()->CreateSample();
	update_matrices = Profiler::instance()->CreateSample();
	gather_instances = Profiler::instance()->CreateSample();
	update_pipeline = Profiler::instance()->CreateSample();
	stat_and_output = Profiler::instance()->CreateSample();
}

Pipeline::Samples::~Samples() {
	Profiler::instance()->ReleaseSample(switch_state);
	Profiler::instance()->ReleaseSample(update_ubo);
	Profiler::instance()->ReleaseSample(update_tbo);
	Profiler::instance()->ReleaseSample(draw_call);
	Profiler::instance()->ReleaseSample(update_offset);
	Profiler::instance()->ReleaseSample(update_matrices);
	Profiler::instance()->ReleaseSample(gather_instances);
	Profiler::instance()->ReleaseSample(update_pipeline);
	Profiler::instance()->ReleaseSample(stat_and_output);
}

void Pipeline::Samples::Reset() {
	switch_state->Reset();

	update_ubo->Reset();
	update_matrices->Reset();
}
