#include "time2.h"
#include "world.h"
#include "camera.h"
#include "resources.h"
#include "tools/math2.h"
#include "tools/random.h"
#include "builtinproperties.h"
#include "particlesysteminternal.h"

IParticleEmitter::IParticleEmitter(void* d) : IObject(d) {}
void IParticleEmitter::Emit(Particle** particles, uint& count) { _suede_dptr()->Emit(particles, count); }
void IParticleEmitter::SetRate(uint value) { _suede_dptr()->SetRate(value); }
uint IParticleEmitter::GetRate() { return _suede_dptr()->GetRate(); }
void IParticleEmitter::SetStartDuration(float value) { _suede_dptr()->SetStartDuration(value); }
float IParticleEmitter::GetStartDuration() { return _suede_dptr()->GetStartDuration(); }
void IParticleEmitter::SetStartSize(float value) { _suede_dptr()->SetStartSize(value); }
float IParticleEmitter::GetStartSize() { return _suede_dptr()->GetStartSize(); }
void IParticleEmitter::SetStartVelocity(const glm::vec3& value) { _suede_dptr()->SetStartVelocity(value); }
glm::vec3 IParticleEmitter::GetStartVelocity() { return _suede_dptr()->GetStartVelocity(); }
void IParticleEmitter::SetStartColor(const glm::vec4& value) { return _suede_dptr()->SetStartColor(value); }
glm::vec4 IParticleEmitter::GetStartColor() { return _suede_dptr()->GetStartColor(); }
void IParticleEmitter::AddBurst(const ParticleBurst& value) { _suede_dptr()->AddBurst(value); }
void IParticleEmitter::SetBurst(int i, const ParticleBurst& value) { _suede_dptr()->SetBurst(i, value); }
ParticleBurst IParticleEmitter::GetBurst(int i) { return _suede_dptr()->GetBurst(i); }
void IParticleEmitter::RemoveBurst(int i) { _suede_dptr()->RemoveBurst(i); }
int IParticleEmitter::GetBurstCount() { return _suede_dptr()->GetBurstCount(); }

ISphereParticleEmitter::ISphereParticleEmitter() : IParticleEmitter(MEMORY_NEW(SphereParticleEmitterInternal, this)) {}
void ISphereParticleEmitter::SetRadius(float value) { _suede_dptr()->SetRadius(value); }
float ISphereParticleEmitter::GetRadius() { return _suede_dptr()->GetRadius(); }

IParticleAnimator::IParticleAnimator() : IObject(MEMORY_NEW(ParticleAnimatorInternal, this)) {}
void IParticleAnimator::SetForce(const glm::vec3& value) { _suede_dptr()->SetForce(value); }
glm::vec3 IParticleAnimator::GetForce() { return _suede_dptr()->GetForce(); }
void IParticleAnimator::SetRandomForce(const glm::vec3& value) { _suede_dptr()->SetRandomForce(value); }
glm::vec3 IParticleAnimator::GetRandomForce() { return _suede_dptr()->GetRandomForce(); }
void IParticleAnimator::SetGravityScale(float value) { _suede_dptr()->SetGravityScale(value); }
float IParticleAnimator::GetGravityScale() { return _suede_dptr()->GetGravityScale(); }
void IParticleAnimator::Update(Particle& particle) { _suede_dptr()->Update(particle); }

IParticleSystem::IParticleSystem() : IComponent(MEMORY_NEW(ParticleSystemInternal, this)) {}
void IParticleSystem::SetDuration(float value) { _suede_dptr()->SetDuration(value); }
float IParticleSystem::GetDuration() { return _suede_dptr()->GetDuration(); }
void IParticleSystem::SetLooping(bool value) { _suede_dptr()->SetLooping(value); }
bool IParticleSystem::GetLooping() { return _suede_dptr()->GetLooping(); }
void IParticleSystem::SetStartDelay(float value) { _suede_dptr()->SetStartDelay(value); }
float IParticleSystem::GetStartDelay() { return _suede_dptr()->GetStartDelay(); }
void IParticleSystem::SetMaxParticles(uint value) { _suede_dptr()->SetMaxParticles(value); }
uint IParticleSystem::GetMaxParticles() { return _suede_dptr()->GetMaxParticles(); }
uint IParticleSystem::GetParticlesCount() const { return _suede_dptr()->GetParticlesCount(); }
const Bounds& IParticleSystem::GetMaxBounds() { return _suede_dptr()->GetMaxBounds(); }
void IParticleSystem::SetEmitter(ParticleEmitter value) { _suede_dptr()->SetEmitter(value); }
ParticleEmitter IParticleSystem::GetEmitter() { return _suede_dptr()->GetEmitter(); }
void IParticleSystem::SetParticleAnimator(ParticleAnimator value) { _suede_dptr()->SetParticleAnimator(value); }
ParticleAnimator IParticleSystem::GetParticleAnimator() { return _suede_dptr()-> GetParticleAnimator(); }

SUEDE_DEFINE_COMPONENT_INTERNAL(ParticleSystem, Component)

static const glm::vec3 kGravitationalAcceleration(0, -9.8f, 0);

#define MAX_PARTICLE_COUNT	1000

ParticleSystemInternal::ParticleSystemInternal(IParticleSystem* self)
	: ComponentInternal(self, ObjectType::ParticleSystem), duration_(3)
	, looping_(false), startDelay_(0), time_(0), maxParticles_(MAX_PARTICLE_COUNT)
	, particles_(MAX_PARTICLE_COUNT), meshDirty_(true), rendererDirty_(true) {
}

ParticleSystemInternal::~ParticleSystemInternal() {
}

void ParticleSystemInternal::Awake() {
	GetGameObject()->AddComponent<MeshFilter>();
	GetGameObject()->AddComponent<ParticleRenderer>();
}

void ParticleSystemInternal::SetMaxParticles(uint value) {
	if (maxParticles_ != value) {
		maxParticles_ = value;
		particles_.reallocate(value);
		meshDirty_ = true;
	}
}

void ParticleSystemInternal::CullingUpdate() {
	if (!meshDirty_ && !rendererDirty_) {
		if (emitter_ && time_ >= startDelay_) {
			UpdateEmitter();
		}

		UpdateParticles();
		time_ += Time::GetDeltaTime();
	}
}

void ParticleSystemInternal::Update() {
	if (meshDirty_) {
		InitializeMesh();
	}

	if (rendererDirty_) {
		InitializeRenderer();
	}

	UpdateInstanceBuffers();
}

void ParticleSystemInternal::SortBuffers() {
	SortParticlesByDepth(CameraUtility::GetMain()->GetTransform()->GetPosition());
}

void ParticleSystemInternal::SortParticlesByDepth(const glm::vec3& ref) {
	uint count = particles_.size();

	// insertion sort particles.
	for (int i = 1; i < count; ++i) {
		glm::vec4 ck = colors_[i];
		glm::vec4 pk = geometries_[i];
		glm::vec3 vk(ref.xyz - pk.xyz);
		float distSquared = glm::dot(vk, vk);

		int j = i - 1;
		for (; j >= 0; --j) {
			const glm::vec4& current = geometries_[j];
			glm::vec3 vc(ref.xyz - current.xyz);
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
	}
}

void ParticleSystemInternal::UpdateInstanceBuffers() {
	uint count = particles_.size();
	if (count > 0) {
		Mesh mesh = GetGameObject()->GetComponent<MeshFilter>()->GetMesh();
		mesh->UpdateInstanceBuffer(0, count * sizeof(glm::vec4), &colors_[0]);
		mesh->UpdateInstanceBuffer(1, count * sizeof(glm::vec4), &geometries_[0]);
	}
}

void ParticleSystemInternal::UpdateAttributes() {
	float deltaTime = Time::GetDeltaTime();

	for (free_list<Particle>::iterator ite = particles_.begin(); ite != particles_.end(); ) {
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

	float maxSize = 0;
	glm::vec3 min(std::numeric_limits<float>::max()), max(std::numeric_limits<float>::lowest());
	for (free_list<Particle>::iterator ite = particles_.begin(); ite != particles_.end(); ++ite) {
		Particle* particle = *ite;

		colors_[index] = particle->color;
		geometries_[index++] = glm::vec4(particle->position, particle->size);

		min = Math::Min(min, particle->position);
		max = Math::Max(max, particle->position);
		maxSize = Math::Max(maxSize, particle->size);
	}

	bounds_.SetMinMax(min, max);
	bounds_.size += maxSize / 2;
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
		buffer_[i]->position += GetTransform()->GetPosition();
	}
}

void ParticleSystemInternal::InitializeMesh() {
	InstanceAttribute color; 
	color.count = maxParticles_, color.divisor = 1;

	InstanceAttribute geometry;
	geometry.count = maxParticles_, geometry.divisor = 1;

	MeshFilter meshFilter = GetGameObject()->GetComponent<MeshFilter>();
	meshFilter->SetMesh(
		Resources::CreateInstancedPrimitive(PrimitiveType::Quad, 1, color, geometry)
	);
	meshDirty_ = false;
}

void ParticleSystemInternal::InitializeRenderer() {
	ParticleRenderer renderer = GetGameObject()->GetComponent<ParticleRenderer>();

	Material material = new IMaterial();
	Shader shader = Resources::FindShader("builtin/particle");
	material->SetShader(shader);

	Texture2D mainTexture = new ITexture2D();
	mainTexture->Load("snowflake.png");
	material->SetTexture(BuiltinProperties::MainTexture, mainTexture);

	renderer->AddMaterial(material);

	rendererDirty_ = false;
}

uint ParticleSystemInternal::GetParticlesCount() const {
	return particles_.size();
}

ParticleEmitterInternal::ParticleEmitterInternal(IParticleEmitter* self, ObjectType type) : ObjectInternal(self, type)
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
			ans = Random::IntRange(bursts_[i].min, bursts_[i].max);
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
