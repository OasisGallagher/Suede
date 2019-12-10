#include <vector>

#include "shader.h"
#include "tools/random.h"
#include "particlesystem.h"
#include "containers/freelist.h"
#include "internal/components/componentinternal.h"

class ParticleEmitterInternal : public ObjectInternal {
public:
	ParticleEmitterInternal(ObjectType type);

	virtual void SetRate(uint value) { rate_ = value; }
	virtual uint GetRate() { return rate_; }

	virtual void SetStartDuration(float value) { startLife_ = value; }
	virtual float GetStartDuration() { return startLife_; }

	virtual void SetStartSize(float value) { startSize_ = value; }
	virtual float GetStartSize() { return startSize_; }

	virtual void SetStartVelocity(const Vector3& value) { startVelocity_ = value; }
	virtual Vector3 GetStartVelocity() { return startVelocity_; }

	virtual void Emit(Particle** particles, uint& count);

	virtual void SetStartColor(const Vector4& value) { startColor_ = value; }
	virtual Vector4 GetStartColor() { return startColor_; }

	virtual void AddBurst(const ParticleBurst& value) { bursts_.push_back(value); }
	virtual void SetBurst(int i, const ParticleBurst& value) { bursts_[i] = value; }
	virtual ParticleBurst GetBurst(int i) { return bursts_[i]; }
	virtual void RemoveBurst(int i) { bursts_.erase(bursts_.begin() + i); }
	virtual int GetBurstCount() { return bursts_.size(); }

protected:
	virtual Vector3 GetStartPosition() { return Vector3(0); }

private:
	uint CalculateNextEmissionParticleCount();
	void EmitParticles(Particle** particles, uint count);

private:
	uint rate_;
	float time_;
	float remainder_;
	float startLife_;
	float startSize_;
	
	Vector4 startColor_;
	Vector3 startVelocity_;
	std::vector<ParticleBurst> bursts_;
};

class SphereParticleEmitterInternal : public ParticleEmitterInternal {
public:
	SphereParticleEmitterInternal() : ParticleEmitterInternal(ObjectType::SphereParticleEmitter) {}

public:
	virtual void SetRadius(float value) { radius_ = value; }
	virtual float GetRadius() { return radius_; }

	virtual Vector3 GetStartPosition() { return Random::InsideSphere(radius_); }

private:
	float radius_;
};

class ParticleAnimatorInternal : public ObjectInternal {
public:
	ParticleAnimatorInternal() : ObjectInternal(ObjectType::ParticleAnimator), gravityScale_(1) {}

public:
	virtual void SetForce(const Vector3& value) { force_ = value; }
	virtual Vector3 GetForce() { return force_; }

	virtual void SetRandomForce(const Vector3& value) { randomForce_ = value; }
	virtual Vector3 GetRandomForce() { return randomForce_; }

	virtual void SetGravityScale(float value) { gravityScale_ = value; }
	virtual float GetGravityScale() { return gravityScale_; }

	virtual void Update(Particle& particle);

private:
	float gravityScale_;

	Vector3 force_;
	Vector3 randomForce_;
};

class ParticleSystemInternal : public ComponentInternal {
public:
	ParticleSystemInternal();
	~ParticleSystemInternal();

public:
	virtual void Awake();

	virtual void SetMaxParticles(uint value);
	virtual uint GetMaxParticles() { return maxParticles_; }

	virtual int GetUpdateStrategy() { return UpdateStrategyCulling | UpdateStrategyRendering; }

	virtual void SetDuration(float value) { duration_ = value; }
	virtual float GetDuration() { return duration_; }

	virtual void SetLooping(bool value) { looping_ = value; }
	virtual bool GetLooping() { return looping_; }

	virtual void SetStartDelay(float value) { startDelay_ = value; }
	virtual float GetStartDelay() { return startDelay_; }

	virtual uint GetParticlesCount() const;
	virtual const Bounds& GetMaxBounds() { return bounds_; }

	virtual void SetEmitter(ParticleEmitter value) { emitter_ = value; }
	virtual ParticleEmitter GetEmitter() { return emitter_; }

	virtual void SetParticleAnimator(ParticleAnimator value) { particleAnimator_ = value; }
	virtual ParticleAnimator GetParticleAnimator() { return particleAnimator_; }

public:
	virtual void Update();
	virtual void CullingUpdate();

private:
	void InitializeMesh();
	void InitializeRenderer();

	void UpdateEmitter();

	void EmitParticles(uint count);

	void SortBuffers();

	void SortParticlesByDepth(const Vector3& ref);

	void UpdateParticles();

	void UpdateInstanceBuffers();

	void UpdateAttributes();
	void UpdateBuffers();

private:
	bool looping_;
	uint maxParticles_;
	float time_;
	float duration_;
	float startDelay_;

	bool meshDirty_;
	bool rendererDirty_;

	Bounds bounds_;

	ParticleEmitter emitter_;
	ParticleAnimator particleAnimator_;

	std::vector<Vector4> colors_;
	std::vector<Vector4> geometries_;
	
	free_list<Particle> particles_;
	std::vector<Particle*> buffer_;
};
