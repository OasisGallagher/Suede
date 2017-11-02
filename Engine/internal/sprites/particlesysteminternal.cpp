#include "variables.h"
#include "tools/mathf.h"
#include "particlesysteminternal.h"
#include "internal/misc/timefinternal.h"
#include "internal/base/shaderinternal.h"
#include "internal/base/textureinternal.h"
#include "internal/base/surfaceinternal.h"
#include "internal/base/materialinternal.h"
#include "internal/base/rendererinternal.h"

static const int quadIndexes[] = { 0, 1, 2, 3 };
static const glm::vec3 quadVertices[] = {
	glm::vec3(-0.5f, -0.5f, 0.f),
	glm::vec3(0.5f, -0.5f, 0.f),
	glm::vec3(-0.5f,  0.5f, 0.f),
	glm::vec3(0.5f,  0.5f, 0.f),
};

#define MAX_PARTICLE_COUNT	1000
#define GRAVITATIONAL_ACCELERATION	9.8f

ParticleSystemInternal::ParticleSystemInternal()
	: SpriteInternal(ObjectTypeParticleSystem), duration_(3)
	, looping_(false), startDelay_(0), time_(0), gravityScale_(1)
	, maxParticles_(MAX_PARTICLE_COUNT), particles_(MAX_PARTICLE_COUNT) {
	InitializeSurface();
	InitializeRenderer();
}

ParticleSystemInternal::~ParticleSystemInternal() {
}

void ParticleSystemInternal::SetMaxParticles(unsigned value) {
	if (maxParticles_ != value) {
		maxParticles_ = value;
		particles_.reallocate(value);
	}
}

void ParticleSystemInternal::Update() {
	UpdateParticles();

	if (emitter_ && time_ >= startDelay_) {
		UpdateEmitter();
	}

	time_ += timeInstance->GetDeltaTime();
}

void ParticleSystemInternal::SortParticles() {
}

void ParticleSystemInternal::UpdateParticles() {
	size_t count = particles_.size();
	if (count != 0) {
		UpdateAttributes();
		SortParticles();

		count = particles_.size();
		Surface surface = GetSurface();
		surface->UpdateUserBuffer(0, count * sizeof(glm::vec4), &colors_[0]);
		surface->UpdateUserBuffer(1, count * sizeof(glm::vec4), &positions_[0]);
	}
}

void ParticleSystemInternal::UpdateAttributes() {
	float deltaTime = timeInstance->GetDeltaTime();

	unsigned index = 0;
	unsigned count = Mathf::NextPowerOfTwo(particles_.size());

	if (colors_.size() < count) { colors_.resize(count); }
	if (positions_.size() < count) { positions_.resize(count); }

	for (free_list<Particle>::iterator ite = particles_.begin(); ite != particles_.end(); ) {
		Particle* particle = *ite++;

		if ((particle->life -= deltaTime) <= 0) {
			particles_.push(particle);
		}
		else {
			particle->position += particle->velocity * deltaTime;
			particle->velocity -= GRAVITATIONAL_ACCELERATION * gravityScale_ * deltaTime;

			colors_[index] = particle->color;
			positions_[index++] = glm::vec4(particle->position, particle->size);
		}
	}
}

void ParticleSystemInternal::UpdateEmitter() {
	unsigned maxCount = Mathf::Max(0, int(GetMaxParticles() - GetParticlesCount()));
	if (maxCount == 0) {
		return;
	}

	unsigned count = 0;
	emitter_->Emit(nullptr, count);
	count = Mathf::Min(count, maxCount);

	if (count != 0) {
		EmitParticles(count);
	}
}

void ParticleSystemInternal::EmitParticles(unsigned count) {
	unsigned size = Mathf::NextPowerOfTwo(count);
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
	mesh->SetPrimaryType(PrimaryTypeTriangleStripe);

	Texture2D albedo = CREATE_OBJECT(Texture2D);
	albedo->Load("textures/snowflake.png");
	mesh->GetMaterialTextures().albedo = albedo;

	SurfaceAttribute attribute;
	attribute.indexes.assign(quadIndexes, quadIndexes + CountOf(quadIndexes));
	// vertices.
	attribute.positions.assign(quadVertices, quadVertices + CountOf(quadVertices));

	// colors.
	attribute.user0.resize(maxParticles_);

	// positions.
	attribute.user1.resize(maxParticles_);

	surface->SetAttribute(attribute);
	mesh->SetTriangles(CountOf(quadVertices), 0, 0);

	surface->AddMesh(mesh);
	SetSurface(surface);
}

void ParticleSystemInternal::InitializeRenderer() {
	ParticleRenderer renderer = CREATE_OBJECT(ParticleRenderer);

	Material material = CREATE_OBJECT(Material);
	Shader shader = CREATE_OBJECT(Shader);
	shader->Load("buildin/shaders/particle");
	material->SetShader(shader);

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
			ans = Mathf::Random(bursts_[i].min, bursts_[i].max);
		}
	}

	remainder_ += ans * timeInstance->GetDeltaTime();
	ans = (unsigned)remainder_;
	remainder_ -= ans;
	return ans;
}

void ParticleAnimatorInternal::Animate(Particle& particle) {

}
