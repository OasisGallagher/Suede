#include "variables.h"
#include "tools/math2.h"
#include "particlesysteminternal.h"
#include "internal/misc/timefinternal.h"
#include "internal/world/worldinternal.h"
#include "internal/base/shaderinternal.h"
#include "internal/base/textureinternal.h"
#include "internal/base/surfaceinternal.h"
#include "internal/base/materialinternal.h"
#include "internal/base/rendererinternal.h"

static const int kQuadIndexes[] = { 0, 1, 2, 3 };
static const glm::vec3 kQuadVertices[] = {
	glm::vec3(-0.5f, -0.5f, 0.f),
	glm::vec3(0.5f, -0.5f, 0.f),
	glm::vec3(-0.5f,  0.5f, 0.f),
	glm::vec3(0.5f,  0.5f, 0.f),
};
static const glm::vec3 kGravitationalAcceleration(0, -9.8f, 0);

#define MAX_PARTICLE_COUNT	1000

ParticleSystemInternal::ParticleSystemInternal()
	: SpriteInternal(ObjectTypeParticleSystem), duration_(3)
	, looping_(false), startDelay_(0), time_(0), maxParticles_(MAX_PARTICLE_COUNT)
	, particles_(MAX_PARTICLE_COUNT) {
	InitializeSurface();
	InitializeRenderer();
}

ParticleSystemInternal::~ParticleSystemInternal() {
}

void ParticleSystemInternal::SetMaxParticles(unsigned value) {
	if (maxParticles_ != value) {
		maxParticles_ = value;
		particles_.reallocate(value);
		InitializeSurface();
	}
}

void ParticleSystemInternal::Update() {
	UpdateParticles();

	if (emitter_ && time_ >= startDelay_) {
		UpdateEmitter();
	}

	time_ += timeInstance->GetDeltaTime();
}

void ParticleSystemInternal::SortBuffers() {
	std::vector<Sprite> cameras;
	if (worldInstance->GetSprites(ObjectTypeCamera, cameras)) {
		SortParticlesByDepth(cameras.front()->GetPosition());
	}
}

void ParticleSystemInternal::SortParticlesByDepth(const glm::vec3& ref) {
	unsigned count = particles_.size();
	for (int i = 1; i < count; ++i) {
		glm::vec4 ck = colors_[i];
		glm::vec4 pk = positions_[i];
		glm::vec3 vk(ref.x - pk.x, ref.y - pk.y, ref.z - pk.z);
		float distSquared = glm::dot(vk, vk);

		int j = i - 1;
		for (; j >= 0; --j) {
			const glm::vec4& current = positions_[j];
			glm::vec3 vc(ref.x - current.x, ref.y - current.y, ref.z - current.z);

			if (glm::dot(vc, vc) >= distSquared) {
				break;
			}
		}

		std::vector<glm::vec4>::iterator pc = colors_.begin(), pp = positions_.begin();
		std::copy_backward(pc + (j + 1), pc + i, pc + (i + 1));
		std::copy_backward(pp + (j + 1), pp + i, pp + (i + 1));
		colors_[j + 1] = ck;
		positions_[j + 1] = pk;
	}
}

void ParticleSystemInternal::UpdateParticles() {
	size_t count = particles_.size();
	if (count != 0) {
		UpdateAttributes();
		UpdateBuffers();
		SortBuffers();
		UpdateSurface();
	}
}

void ParticleSystemInternal::UpdateSurface() {
	unsigned count = particles_.size();
	Surface surface = GetSurface(0);
	surface->UpdateUserBuffer(0, count * sizeof(glm::vec4), &colors_[0]);
	surface->UpdateUserBuffer(1, count * sizeof(glm::vec4), &positions_[0]);
}

void ParticleSystemInternal::UpdateAttributes() {
	float deltaTime = timeInstance->GetDeltaTime();

	for (free_list<Particle>::iterator ite = particles_.begin(); ite != particles_.end(); ) {
		Particle* particle = *ite++;

		if ((particle->life -= deltaTime) <= 0) {
			particles_.push(particle);
		}
		else if (particleAnimator_) {
			particleAnimator_->Update(*particle);
		}
	}
}

void ParticleSystemInternal::UpdateBuffers() {
	unsigned index = 0;
	unsigned count = Math::NextPowerOfTwo(particles_.size());

	if (colors_.size() < count) { colors_.resize(count); }
	if (positions_.size() < count) { positions_.resize(count); }

	for (free_list<Particle>::iterator ite = particles_.begin(); ite != particles_.end(); ++ite) {
		Particle* particle = *ite;

		colors_[index] = particle->color;
		positions_[index++] = glm::vec4(particle->position, particle->size);
	}
}

void ParticleSystemInternal::UpdateEmitter() {
	unsigned maxCount = Math::Max(0, int(GetMaxParticles() - GetParticlesCount()));
	if (maxCount == 0) {
		return;
	}

	unsigned count = 0;
	emitter_->Emit(nullptr, count);
	count = Math::Min(count, maxCount);

	if (count != 0) {
		EmitParticles(count);
	}
}

void ParticleSystemInternal::EmitParticles(unsigned count) {
	unsigned size = Math::NextPowerOfTwo(count);
	if (size > buffer_.size()) {
		buffer_.resize(size);
	}

	for (int i = 0; i < count; ++i) {
		buffer_[i] = particles_.pop();
	}

	emitter_->Emit(&buffer_[0], count);
	for (int i = 0; i < count; ++i) {
		buffer_[i]->position += GetPosition();
	}
}

void ParticleSystemInternal::InitializeSurface() {
	Mesh mesh = CREATE_OBJECT(Mesh);
	Surface surface = CREATE_OBJECT(Surface);
	mesh->SetTopology(MeshTopologyTriangles);

	SurfaceAttribute attribute;
	attribute.indexes.assign(kQuadIndexes, kQuadIndexes + CountOf(kQuadIndexes));
	// vertices.
	attribute.positions.assign(kQuadVertices, kQuadVertices + CountOf(kQuadVertices));

	// colors.
	attribute.user0.divisor = 1;
	attribute.user0.data.resize(maxParticles_);

	// positions.
	attribute.user1.divisor = 1;
	attribute.user1.data.resize(maxParticles_);

	surface->SetAttribute(attribute);
	mesh->SetTriangles(CountOf(kQuadVertices), 0, 0);

	surface->AddMesh(mesh);
	AddSurface(surface);
}

void ParticleSystemInternal::InitializeRenderer() {
	ParticleRenderer renderer = CREATE_OBJECT(ParticleRenderer);

	Material material = CREATE_OBJECT(Material);
	Shader shader = CREATE_OBJECT(Shader);
	shader->Load("buildin/shaders/particle");
	material->SetShader(shader);

	Texture2D mainTexture = CREATE_OBJECT(Texture2D);
	mainTexture->Load("textures/snowflake.png");
	material->SetTexture(Variables::mainTexture, mainTexture);

	renderer->AddMaterial(material);
	SetRenderer(renderer);
}

unsigned ParticleSystemInternal::GetParticlesCount() {
	return particles_.size();
}

ParticleEmitterInternal::ParticleEmitterInternal(ObjectType type) : ObjectInternal(type)
	, rate_(1), time_(-1), remainder_(0) {
}

void ParticleEmitterInternal::Emit(Particle** particles, unsigned& count) {
	if (particles == nullptr) {
		count = CalculateNextEmissionParticleCount();
		return;
	}

	if (time_ < 0) { time_ = 0; }

	EmitParticles(particles, count);
}

void ParticleEmitterInternal::EmitParticles(Particle** particles, unsigned count) {
	for (unsigned i = 0; i < count; ++i) {
		Particle* item = particles[i];
		item->life = GetStartDuration();
		item->size = GetStartSize();
		item->velocity = GetStartVelocity();

		item->color = GetStartColor();
		item->position = GetStartPosition();
	}
}

unsigned ParticleEmitterInternal::CalculateNextEmissionParticleCount() {
	unsigned ans = rate_;
	float nextTime = time_ + timeInstance->GetDeltaTime();
	for (int i = 0; i < bursts_.size(); ++i) {
		if (bursts_[i].time > time_ && bursts_[i].time <= nextTime) {
			ans = Math::Random(bursts_[i].min, bursts_[i].max);
		}
	}

	remainder_ += ans * timeInstance->GetDeltaTime();
	ans = (unsigned)remainder_;
	remainder_ -= ans;
	return ans;
}

void ParticleAnimatorInternal::Update(Particle& particle) {
	float deltaTime = timeInstance->GetDeltaTime();
	particle.position += particle.velocity * deltaTime;
	particle.velocity += kGravitationalAcceleration * gravityScale_ * deltaTime;
}
