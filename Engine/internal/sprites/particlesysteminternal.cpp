#include "math2.h"
#include "time2.h"
#include "variables.h"
#include "resources.h"
#include "particlesysteminternal.h"
#include "internal/base/meshinternal.h"
#include "internal/world/worldinternal.h"
#include "internal/base/shaderinternal.h"
#include "internal/base/textureinternal.h"
#include "internal/base/materialinternal.h"
#include "internal/base/rendererinternal.h"

static const glm::vec3 kGravitationalAcceleration(0, -9.8f, 0);

#define MAX_PARTICLE_COUNT	1000

ParticleSystemInternal::ParticleSystemInternal()
	: SpriteInternal(ObjectTypeParticleSystem), duration_(3)
	, looping_(false), startDelay_(0), time_(0), maxParticles_(MAX_PARTICLE_COUNT)
	, particles_(MAX_PARTICLE_COUNT) {
	InitializeMesh();
	InitializeRenderer();
}

ParticleSystemInternal::~ParticleSystemInternal() {
}

void ParticleSystemInternal::SetMaxParticles(uint value) {
	if (maxParticles_ != value) {
		maxParticles_ = value;
		particles_.reallocate(value);
		InitializeMesh();
	}
}

void ParticleSystemInternal::Update() {
	UpdateParticles();

	if (emitter_ && time_ >= startDelay_) {
		UpdateEmitter();
	}

	time_ += Time::GetDeltaTime();
}

void ParticleSystemInternal::SortBuffers() {
	std::vector<Sprite> cameras;
	if (worldInstance->GetSprites(ObjectTypeCamera, cameras)) {
		SortParticlesByDepth(cameras.front()->GetPosition());
	}
}

void ParticleSystemInternal::SortParticlesByDepth(const glm::vec3& ref) {
	uint count = particles_.size();
	for (int i = 1; i < count; ++i) {
		glm::vec4 ck = colors_[i];
		glm::vec4 pk = geometries_[i];
		glm::vec3 vk(ref.x - pk.x, ref.y - pk.y, ref.z - pk.z);
		float distSquared = glm::dot(vk, vk);

		int j = i - 1;
		for (; j >= 0; --j) {
			const glm::vec4& current = geometries_[j];
			glm::vec3 vc(ref.x - current.x, ref.y - current.y, ref.z - current.z);

			if (glm::dot(vc, vc) >= distSquared) {
				break;
			}
		}

		std::vector<glm::vec4>::iterator pc = colors_.begin(), pp = geometries_.begin();
		std::copy_backward(pc + (j + 1), pc + i, pc + (i + 1));
		std::copy_backward(pp + (j + 1), pp + i, pp + (i + 1));
		colors_[j + 1] = ck;
		geometries_[j + 1] = pk;
	}
}

void ParticleSystemInternal::UpdateParticles() {
	size_t count = particles_.size();
	if (count != 0) {
		UpdateAttributes();
		UpdateBuffers();
		SortBuffers();
		UpdateMesh();
	}
}

void ParticleSystemInternal::UpdateMesh() {
	uint count = particles_.size();
	if (count > 0) {
		Mesh mesh = GetMesh();
		mesh->UpdateInstanceBuffer(0, count * sizeof(glm::vec4), &colors_[0]);
		mesh->UpdateInstanceBuffer(1, count * sizeof(glm::vec4), &geometries_[0]);
	}
}

void ParticleSystemInternal::UpdateAttributes() {
	float deltaTime = Time::GetDeltaTime();

	for (FreeList<Particle>::iterator ite = particles_.begin(); ite != particles_.end(); ) {
		Particle* particle = *ite++;

		if ((particle->life -= deltaTime) <= 0) {
			particles_.recycle(particle);
		}
		else if (particleAnimator_) {
			particleAnimator_->Update(*particle);
		}
	}
}

void ParticleSystemInternal::UpdateBuffers() {
	uint index = 0;
	uint count = Math::NextPowerOfTwo(particles_.size());

	if (colors_.size() < count) { colors_.resize(count); }
	if (geometries_.size() < count) { geometries_.resize(count); }

	for (FreeList<Particle>::iterator ite = particles_.begin(); ite != particles_.end(); ++ite) {
		Particle* particle = *ite;

		colors_[index] = particle->color;
		geometries_[index++] = glm::vec4(particle->position, particle->size);
	}
}

void ParticleSystemInternal::UpdateEmitter() {
	uint maxCount = Math::Max(0, int(GetMaxParticles() - GetParticlesCount()));
	if (maxCount == 0) {
		return;
	}

	uint count = 0;
	emitter_->Emit(nullptr, count);
	count = Math::Min(count, maxCount);

	if (count != 0) {
		EmitParticles(count);
	}
}

void ParticleSystemInternal::EmitParticles(uint count) {
	uint size = Math::NextPowerOfTwo(count);
	if (size > buffer_.size()) {
		buffer_.resize(size);
	}

	for (int i = 0; i < count; ++i) {
		buffer_[i] = particles_.spawn();
	}

	emitter_->Emit(&buffer_[0], count);
	for (int i = 0; i < count; ++i) {
		buffer_[i]->position += GetPosition();
	}
}

void ParticleSystemInternal::InitializeMesh() {
	InstanceAttribute color{ maxParticles_, 1 };
	InstanceAttribute geometry{ maxParticles_, 1 };
	Mesh mesh = Resources::CreateInstancedPrimitive(PrimitiveTypeQuad, 1, color, geometry);
	SetMesh(mesh);
}

void ParticleSystemInternal::InitializeRenderer() {
	ParticleRenderer renderer = CREATE_OBJECT(ParticleRenderer);

	Material material = CREATE_OBJECT(Material);
	Shader shader = Resources::FindShader("buildin/shaders/particle");
	material->SetShader(shader);

	Texture2D mainTexture = CREATE_OBJECT(Texture2D);
	mainTexture->Load("textures/snowflake.png");
	material->SetTexture(Variables::mainTexture, mainTexture);

	renderer->AddMaterial(material);
	SetRenderer(renderer);
}

uint ParticleSystemInternal::GetParticlesCount() {
	return particles_.size();
}

ParticleEmitterInternal::ParticleEmitterInternal(ObjectType type) : ObjectInternal(type)
	, rate_(1), time_(-1), remainder_(0) {
}

void ParticleEmitterInternal::Emit(Particle** particles, uint& count) {
	if (particles == nullptr) {
		count = CalculateNextEmissionParticleCount();
		return;
	}

	if (time_ < 0) { time_ = 0; }

	EmitParticles(particles, count);
}

void ParticleEmitterInternal::EmitParticles(Particle** particles, uint count) {
	for (uint i = 0; i < count; ++i) {
		Particle* item = particles[i];
		item->life = GetStartDuration();
		item->size = GetStartSize();
		item->velocity = GetStartVelocity();

		item->color = GetStartColor();
		item->position = GetStartPosition();
	}
}

uint ParticleEmitterInternal::CalculateNextEmissionParticleCount() {
	uint ans = rate_;
	float nextTime = time_ + Time::GetDeltaTime();
	for (int i = 0; i < bursts_.size(); ++i) {
		if (bursts_[i].time > time_ && bursts_[i].time <= nextTime) {
			ans = Math::Random(bursts_[i].min, bursts_[i].max);
		}
	}

	remainder_ += ans * Time::GetDeltaTime();
	ans = (uint)remainder_;
	remainder_ -= ans;
	return ans;
}

void ParticleAnimatorInternal::Update(Particle& particle) {
	float deltaTime = Time::GetDeltaTime();
	particle.position += particle.velocity * deltaTime;
	particle.velocity += kGravitationalAcceleration * gravityScale_ * deltaTime;
}
