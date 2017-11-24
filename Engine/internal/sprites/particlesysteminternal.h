#include <vector>

#include "shader.h"
#include "tools/math2.h"
#include "particlesystem.h"
#include "internal/containers/freelist.h"
#include "internal/sprites/spriteinternal.h"

class ParticleEmitterInternal : virtual public IParticleEmitter, public ObjectInternal {
public:
	ParticleEmitterInternal(ObjectType type);

	virtual void SetRate(uint value) { rate_ = value; }
	virtual uint GetRate() { return rate_; }

	virtual void SetStartDuration(float value) { startLife_ = value; }
	virtual float GetStartDuration() { return startLife_; }

	virtual void SetStartSize(float value) { startSize_ = value; }
	virtual float GetStartSize() { return startSize_; }

	virtual void SetStartVelocity(const glm::vec3& value) { startVelocity_ = value; }
	virtual glm::vec3 GetStartVelocity() { return startVelocity_; }

	virtual void Emit(Particle** particles, uint& count);

	virtual void SetStartColor(const glm::vec4& value) { startColor_ = value; }
	virtual glm::vec4 GetStartColor() { return startColor_; }

	virtual void AddBurst(const ParticleBurst& value) { bursts_.push_back(value); }
	virtual void SetBurst(int i, const ParticleBurst& value) { bursts_[i] = value; }
	virtual ParticleBurst GetBurst(int i) { return bursts_[i]; }
	virtual void RemoveBurst(int i) { bursts_.erase(bursts_.begin() + i); }
	virtual int GetBurstCount() { return bursts_.size(); }

protected:
	virtual glm::vec3 GetStartPosition() { return glm::vec3(0); }

private:
	uint CalculateNextEmissionParticleCount();
	void EmitParticles(Particle** particles, uint count);

private:
	uint rate_;
	float time_;
	float remainder_;
	float startLife_;
	float startSize_;
	
	glm::vec4 startColor_;
	glm::vec3 startVelocity_;
	std::vector<ParticleBurst> bursts_;
};

class SphereParticleEmitterInternal : public ISphereParticleEmitter, public ParticleEmitterInternal {
	DEFINE_FACTORY_METHOD(SphereParticleEmitter)

public:
	SphereParticleEmitterInternal() : ParticleEmitterInternal(ObjectTypeSphereParticleEmitter) {}

public:
	virtual void SetRadius(float value) { radius_ = value; }
	virtual float GetRadius() { return radius_; }

	virtual glm::vec3 GetStartPosition() { return Math::RandomInsideSphere(radius_); }

private:
	float radius_;
};

class ParticleAnimatorInternal : public IParticleAnimator, public ObjectInternal {
	DEFINE_FACTORY_METHOD(ParticleAnimator)

public:
	ParticleAnimatorInternal() : ObjectInternal(ObjectTypeParticleAnimator), gravityScale_(1) {}
public:
	virtual void SetForce(const glm::vec3& value) { force_ = value; }
	virtual glm::vec3 GetForce() { return force_; }

	virtual void SetRandomForce(const glm::vec3& value) { randomForce_ = value; }
	virtual glm::vec3 GetRandomForce() { return randomForce_; }

	virtual void SetGravityScale(float value) { gravityScale_ = value; }
	virtual float GetGravityScale() { return gravityScale_; }

	virtual void Update(Particle& particle);

private:
	float gravityScale_;

	glm::vec3 force_;
	glm::vec3 randomForce_;
};

class ParticleSystemInternal : public IParticleSystem, public SpriteInternal {
	DEFINE_FACTORY_METHOD(ParticleSystem)

public:
	ParticleSystemInternal();
	~ParticleSystemInternal();

public:
	virtual void SetMaxParticles(uint value);
	virtual uint GetMaxParticles() { return maxParticles_; }

	virtual void SetDuration(float value) { duration_ = value; }
	virtual float GetDuration() { return duration_; }

	virtual void SetLooping(bool value) { looping_ = value; }
	virtual bool GetLooping() { return looping_; }

	virtual void SetStartDelay(float value) { startDelay_ = value; }
	virtual float GetStartDelay() { return startDelay_; }

	virtual uint GetParticlesCount();

	virtual void SetEmitter(ParticleEmitter value) { emitter_ = value; }
	virtual ParticleEmitter GetEmitter() { return emitter_; }

	virtual void SetParticleAnimator(ParticleAnimator value) { particleAnimator_ = value; }
	virtual ParticleAnimator GetParticleAnimator() { return particleAnimator_; }

public:
	virtual void Update();

private:
	void InitializeMesh();
	void InitializeRenderer();

	void UpdateEmitter();

	void EmitParticles(uint count);

	void SortBuffers();

	void SortParticlesByDepth(const glm::vec3& ref);

	void UpdateParticles();

	void UpdateMesh();

	void UpdateAttributes();
	void UpdateBuffers();

private:
	bool looping_;
	uint maxParticles_;
	float time_;
	float duration_;
	float startDelay_;

	ParticleEmitter emitter_;
	ParticleAnimator particleAnimator_;

	std::vector<glm::vec4> colors_;
	std::vector<glm::vec4> geometries_;
	
	FreeList<Particle> particles_;
	std::vector<Particle*> buffer_;
};
