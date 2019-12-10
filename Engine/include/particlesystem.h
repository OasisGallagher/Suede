#pragma once
#include "component.h"

struct ParticleBurst {
	float time;
	uint min;
	uint max;
};

struct Particle {
	float life;
	Vector3 velocity;

	float size;
	Vector4 color;
	Vector3 position;
};

SUEDE_DEFINE_OBJECT_POINTER(ParticleEmitter)
SUEDE_DEFINE_OBJECT_POINTER(SphereParticleEmitter)
SUEDE_DEFINE_OBJECT_POINTER(ParticleAnimator)

class SUEDE_API IParticleEmitter : public IObject {
	SUEDE_DEFINE_METATABLE_NAME(ParticleEmitter)
	SUEDE_DECLARE_IMPLEMENTATION(ParticleEmitter)

public:
	void Emit(Particle** particles, uint& count);

	void SetRate(uint value);
	uint GetRate();

	void SetStartDuration(float value);
	float GetStartDuration();

	void SetStartSize(float value);
	float GetStartSize();

	void SetStartVelocity(const Vector3& value);
	Vector3 GetStartVelocity();

	void SetStartColor(const Vector4& value);
	Vector4 GetStartColor();

	void AddBurst(const ParticleBurst& value);
	void SetBurst(int i, const ParticleBurst& value);
	ParticleBurst GetBurst(int i);
	void RemoveBurst(int i);
	int GetBurstCount();

protected:
	IParticleEmitter(void* d);
};

class SUEDE_API ISphereParticleEmitter : public IParticleEmitter {
	SUEDE_DEFINE_METATABLE_NAME(SphereParticleEmitter)
	SUEDE_DECLARE_IMPLEMENTATION(SphereParticleEmitter)

public:
	ISphereParticleEmitter();

public:
	void SetRadius(float value);
	float GetRadius();
};

class SUEDE_API IParticleAnimator : public IObject {
	SUEDE_DEFINE_METATABLE_NAME(ParticleEmitter)
	SUEDE_DECLARE_IMPLEMENTATION(ParticleAnimator)

public:
	IParticleAnimator();

public:
	void SetForce(const Vector3& value);
	Vector3 GetForce();

	void SetRandomForce(const Vector3& value);
	Vector3 GetRandomForce();

	void SetGravityScale(float value);
	float GetGravityScale();

	void Update(Particle& particle);
};

class SUEDE_API IParticleSystem : public IComponent {
	SUEDE_DECLARE_COMPONENT()
	SUEDE_DEFINE_METATABLE_NAME(ParticleSystem)
	SUEDE_DECLARE_IMPLEMENTATION(ParticleSystem)

public:
	IParticleSystem();

public:
	void SetDuration(float value);
	float GetDuration();

	void SetLooping(bool value);
	bool GetLooping();

	void SetStartDelay(float value);
	float GetStartDelay();

	void SetMaxParticles(uint value);
	uint GetMaxParticles();

	uint GetParticlesCount() const;
	const Bounds& GetMaxBounds();

	void SetEmitter(ParticleEmitter value);
	ParticleEmitter GetEmitter();

	void SetParticleAnimator(ParticleAnimator value);
	ParticleAnimator GetParticleAnimator();
};

SUEDE_DEFINE_OBJECT_POINTER(ParticleSystem)
