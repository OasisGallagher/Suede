#pragma once
#include "sprite.h"

struct ParticleBurst {
	float time;
	unsigned min;
	unsigned max;
};

struct Particle {
	float life;
	glm::vec3 velocity;

	float size;
	glm::vec4 color;
	glm::vec3 position;
};

class IParticleEmitter;
class ISphereParticleEmitter;
class IParticleAnimator;

typedef std::shared_ptr<IParticleEmitter> ParticleEmitter;
typedef std::shared_ptr<ISphereParticleEmitter> SphereParticleEmitter;
typedef std::shared_ptr<IParticleAnimator> ParticleAnimator;

class IParticleEmitter : virtual public IObject {
public:
	virtual void Emit(Particle** particles, unsigned& count) = 0;

	virtual void SetRate(unsigned value) = 0;
	virtual unsigned GetRate() = 0;

	virtual void SetStartDuration(float value) = 0;
	virtual float GetStartDuration() = 0;

	virtual void SetStartSize(float value) = 0;
	virtual float GetStartSize() = 0;

	virtual void SetStartVelocity(const glm::vec3& value) = 0;
	virtual glm::vec3 GetStartVelocity() = 0;

	virtual void SetStartColor(const glm::vec4& value) = 0;
	virtual glm::vec4 GetStartColor() = 0;

	virtual void AddBurst(const ParticleBurst& value) = 0;
	virtual void SetBurst(int i, const ParticleBurst& value) = 0;
	virtual ParticleBurst GetBurst(int i) = 0;
	virtual void RemoveBurst(int i) = 0;
	virtual int GetBurstCount() = 0;
};

class ISphereParticleEmitter : virtual public IParticleEmitter {
public:
	virtual void SetRadius(float value) = 0;
	virtual float GetRadius() = 0;
};

class IParticleAnimator : virtual public IObject {
public:
	virtual void SetForce(const glm::vec3& value) = 0;
	virtual glm::vec3 GetForce() = 0;

	virtual void SetRandomForce(const glm::vec3& value) = 0;
	virtual glm::vec3 GetRandomForce() = 0;

	virtual void Animate(Particle& particle) = 0;
};

class ENGINE_EXPORT IParticleSystem : virtual public ISprite {
public:
	virtual void SetDuration(float value) = 0;
	virtual float GetDuration() = 0;

	virtual void SetLooping(bool value) = 0;
	virtual bool GetLooping() = 0;

	virtual void SetStartDelay(float value) = 0;
	virtual float GetStartDelay() = 0;

	virtual void SetGravityScale(float value) = 0;

	virtual float GetGravityScale() = 0;

	virtual void SetMaxParticles(unsigned value) = 0;
	virtual unsigned GetMaxParticles() = 0;

	virtual unsigned GetParticlesCount() = 0;

	virtual void SetEmitter(ParticleEmitter value) = 0;
	virtual ParticleEmitter GetEmitter() = 0;

	virtual void SetParticleAnimator(ParticleAnimator value) = 0;
	virtual ParticleAnimator GetParticleAnimator() = 0;
};

typedef std::shared_ptr<IParticleSystem> ParticleSystem;
