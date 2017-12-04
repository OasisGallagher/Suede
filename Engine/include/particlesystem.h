#pragma once
#include "sprite.h"

struct ParticleBurst {
	float time;
	uint min;
	uint max;
};

struct Particle {
	float life;
	glm::vec3 velocity;

	float size;
	glm::vec4 color;
	glm::vec3 position;
};

SUEDE_DEFINE_OBJECT_POINTER(ParticleEmitter);
SUEDE_DEFINE_OBJECT_POINTER(SphereParticleEmitter);
SUEDE_DEFINE_OBJECT_POINTER(ParticleAnimator);

SUEDE_DECLARE_OBJECT_CREATER(SphereParticleEmitter);
SUEDE_DECLARE_OBJECT_CREATER(ParticleAnimator);

class SUEDE_API IParticleEmitter : virtual public IObject {
public:
	virtual void Emit(Particle** particles, uint& count) = 0;

	virtual void SetRate(uint value) = 0;
	virtual uint GetRate() = 0;

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

class SUEDE_API ISphereParticleEmitter : virtual public IParticleEmitter {
public:
	virtual void SetRadius(float value) = 0;
	virtual float GetRadius() = 0;
};

class SUEDE_API IParticleAnimator : virtual public IObject {
public:
	virtual void SetForce(const glm::vec3& value) = 0;
	virtual glm::vec3 GetForce() = 0;

	virtual void SetRandomForce(const glm::vec3& value) = 0;
	virtual glm::vec3 GetRandomForce() = 0;

	virtual void SetGravityScale(float value) = 0;
	virtual float GetGravityScale() = 0;

	virtual void Update(Particle& particle) = 0;
};

class SUEDE_API IParticleSystem : virtual public ISprite {
public:
	virtual void SetDuration(float value) = 0;
	virtual float GetDuration() = 0;

	virtual void SetLooping(bool value) = 0;
	virtual bool GetLooping() = 0;

	virtual void SetStartDelay(float value) = 0;
	virtual float GetStartDelay() = 0;

	virtual void SetMaxParticles(uint value) = 0;
	virtual uint GetMaxParticles() = 0;

	virtual uint GetParticlesCount() = 0;

	virtual void SetEmitter(ParticleEmitter value) = 0;
	virtual ParticleEmitter GetEmitter() = 0;

	virtual void SetParticleAnimator(ParticleAnimator value) = 0;
	virtual ParticleAnimator GetParticleAnimator() = 0;
};

SUEDE_DEFINE_OBJECT_POINTER(ParticleSystem);
SUEDE_DECLARE_OBJECT_CREATER(ParticleSystem);
