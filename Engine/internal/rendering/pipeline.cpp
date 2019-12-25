#include "pipeline.h"

#include <algorithm>

#include "profiler.h"
#include "renderingcontext.h"

#include "internal/base/vertexattrib.h"
#include "internal/base/renderdefines.h"

#include "internal/rendering/renderingcontext.h"

//#define DEBUG_SAMPLES

template <class T>
inline int Compare(T lhs, T rhs) {
	if (lhs == rhs) { return 0; }
	return lhs < rhs ? -1 : 1;
}

typedef bool(*RenderableComparer)(const Renderable& lhs, const Renderable& rhs);

#define COMPARE_RETURN(lhs, rhs)	{ int n = Compare(lhs, rhs); if (n != 0) { return n; } }

static int MeshPredicate(const Renderable& lhs, const Renderable &rhs) {
	COMPARE_RETURN(lhs.mesh->GetNativePointer(), rhs.mesh->GetNativePointer());
	COMPARE_RETURN(lhs.subMeshIndex, rhs.subMeshIndex);

	const TriangleBias& bias = lhs.mesh->GetSubMesh(lhs.subMeshIndex)->GetTriangleBias();
	const TriangleBias& otherBias = rhs.mesh->GetSubMesh(rhs.subMeshIndex)->GetTriangleBias();
	COMPARE_RETURN(bias.indexCount, otherBias.indexCount);
	COMPARE_RETURN(bias.baseIndex, otherBias.baseIndex);
	COMPARE_RETURN(bias.baseVertex, otherBias.baseVertex);

	return 0;
}

static int MaterialPredicate(const Renderable& lhs, const Renderable& rhs) {
	const ref_ptr<Material>& lm = lhs.material, &rm = rhs.material;

	COMPARE_RETURN(lm->GetRenderQueue(), rm->GetRenderQueue());
	COMPARE_RETURN(lm, rm);
	COMPARE_RETURN(lhs.pass, rhs.pass);
	COMPARE_RETURN(lm->GetPassNativePointer(lhs.pass), rm->GetPassNativePointer(rhs.pass));

	return 0;
}

Pipeline::Pipeline(RenderingContext* context)
	: context_(context)
	, renderables_(INIT_RENDERABLE_CAPACITY), matrices_(INIT_RENDERABLE_CAPACITY * 2), nrenderables_(0) {
	memset(&counters_, 0, sizeof(counters_));
	oldStates_.Reset();
	matrixBuffer_ = context_->GetUniformState()->matrixTextureBuffer.get();
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

void Pipeline::Sort(SortMode mode, const Matrix4& worldToClipMatrix) {
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
	Sample* updatePipelineSample = Profiler::CreateSample();
	updatePipelineSample->Start();

	Sample* updateTBOSample = Profiler::CreateSample();
	updateTBOSample->Start();
	UpdateMatrixBuffer(nrenderables_, &matrices_[0]);
	updateTBOSample->Stop();

	targetTexture_->BindWrite(normalizedRect_);

	uint from = 0;

	RenderSamples renderSamples;

	for (std::vector<uint>::iterator ite = ranges_.begin(); ite != ranges_.end(); ++ite) {
		Renderable& first = renderables_[from];
		if (first.material->IsPassEnabled(first.pass)) {
			if (renderables_[from].instance != 0) {
				Render(renderables_[from], renderables_[from].instance, 0, renderSamples);
			}
			else {
				RenderInstances(from, *ite, renderSamples);
			}
		}

		from = *ite;
	}

	targetTexture_->Unbind();

	updatePipelineSample->Stop();

	Sample* statAndOutput = Profiler::CreateSample();
	statAndOutput->Start();
	Statistics* statistics = context_->GetStatistics();
	statistics->AddTriangles(counters_.triangles);
	statistics->AddDrawcalls(counters_.drawcalls);
	

#ifdef DEBUG_SAMPLES
	Debug::OutputToConsole("[Pipeline::Update::renderables]\t%d ms", nrenderables_);
	Debug::OutputToConsole("[Pipeline::Update::drawcalls]\t%d ms", counters_.drawcalls);
	Debug::OutputToConsole("[Pipeline::Update::meshChanges]\t%d ms", counters_.meshChanges);
	Debug::OutputToConsole("[Pipeline::Update::materialChanges]\t%d ms", counters_.materialChanges);

	Debug::OutputToConsole("[Pipeline::Update::update_tbo]\t%.2f ms", updateTBOSample->GetElapsedSeconds() * 1000);
	Debug::OutputToConsole("[Pipeline::Update::update_offset]\t%.2f ms", renderSamples.updateOffset->GetElapsedSeconds() * 1000);
	Debug::OutputToConsole("[Pipeline::Update::draw_call]\t%.2f ms", renderSamples.drawCall->GetElapsedSeconds() * 1000);
	Debug::OutputToConsole("[Pipeline::Update::switch_state]\t%.2f ms", renderSamples.switchState->GetElapsedSeconds() * 1000);
	Debug::OutputToConsole("[Pipeline::Update::update_pipeline]\t%.2f ms", updatePipelineSample->GetElapsedSeconds() * 1000);
#endif

	statAndOutput->Stop();

#ifdef DEBUG_SAMPLES
	Debug::OutputToConsole("[Pipeline::Update::stat_and_output]\t%.2f ms", statAndOutput->GetElapsedSeconds() * 1000);
#endif

	Sample* resetStateSample = Profiler::CreateSample();
	resetStateSample->Start();
	ResetState();
	resetStateSample->Stop();

#ifdef DEBUG_SAMPLES
	Debug::OutputToConsole("[Pipeline::Update::reset_states]\t%.2f ms", resetStateSample->GetElapsedSeconds() * 1000);
#endif

	Profiler::ReleaseSample(updateTBOSample);
	Profiler::ReleaseSample(updatePipelineSample);
	Profiler::ReleaseSample(statAndOutput);
	Profiler::ReleaseSample(resetStateSample);
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

void Pipeline::RenderInstances(uint first, uint last, RenderSamples& samples) {
	Render(renderables_[first], last - first, first * 8, samples);
}

void Pipeline::AddRenderable(Mesh* mesh, uint subMeshIndex, Material* material, uint pass, const Matrix4& localToWorldMatrix, uint instance) {
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

void Pipeline::AddRenderable(Mesh* mesh, Material* material, uint pass, const Matrix4& localToWorldMatrix, uint instance /*= 0 */) {
	for (int i = 0; i < mesh->GetSubMeshCount(); ++i) {
		AddRenderable(mesh, i, material, 0, localToWorldMatrix, instance);
	}
}

void Pipeline::Render(Renderable& renderable, uint instance, uint matrixOffset, RenderSamples& samples) {
	samples.switchState->Start();
	UpdateState(renderable);
	samples.switchState->Stop();

	samples.updateOffset->Start();
	context_->VertexAttribI1i(VertexAttribMatrixTextureBufferOffset, matrixOffset);
	samples.updateOffset->Stop();

	const TriangleBias& bias = renderable.mesh->GetSubMesh(renderable.subMeshIndex)->GetTriangleBias();

	samples.drawCall->Start();
	context_->DrawElementsInstancedBaseVertex(renderable.mesh->GetTopology(), bias, instance);
	samples.drawCall->Stop();

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

void Pipeline::UpdateMatrixBuffer(uint size, const void* data) {
	size *= sizeof(Matrix4) * 2;

	if (size > context_->GetLimit(ContextLimitType::MaxTextureBufferSize)) {
		Debug::LogError("%u exceeds matrix buffer max size %u.", size, context_->GetLimit(ContextLimitType::MaxTextureBufferSize));
		return;
	}

	uint newSize = matrixBuffer_->GetSize();
	for (; size > newSize; newSize *= 2)
		;

	if (newSize > matrixBuffer_->GetSize()) {
		matrixBuffer_->Create(newSize);
	}

	matrixBuffer_->Update(0, size, data);
}

void Pipeline::Clear() {
	ranges_.clear();

	for (uint i = 0; i < nrenderables_; ++i) {
		renderables_[i].Clear();
	}

	nrenderables_ = 0;
}

RenderTexture* Pipeline::GetTargetTexture() {
	return targetTexture_.get();
}

void Pipeline::SetTargetTexture(RenderTexture* value, const Rect& normalizedRect) {
	targetTexture_ = value;
	normalizedRect_ = normalizedRect;
}

void Pipeline::AssignRenderables(const Pipeline* other) {
	ranges_ = other->ranges_;
	matrices_ = other->matrices_;

	renderables_ = other->renderables_;
	nrenderables_ = other->nrenderables_;
}

void Pipeline::ResetState() {
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

Pipeline::RenderSamples::RenderSamples() {
	switchState = Profiler::CreateSample();
	updateOffset = Profiler::CreateSample();
	drawCall = Profiler::CreateSample();
}

Pipeline::RenderSamples::~RenderSamples() {
	Profiler::ReleaseSample(switchState);
	Profiler::ReleaseSample(updateOffset);
	Profiler::ReleaseSample(drawCall);
}
