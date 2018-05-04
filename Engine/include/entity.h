#pragma once
#include "mesh.h"
#include "bounds.h"
#include "renderer.h"
#include "animation.h"
#include "transform.h"
#include "particlesystem.h"

SUEDE_DEFINE_OBJECT_POINTER(Entity);
SUEDE_DECLARE_OBJECT_CREATER(Entity);

enum EntityStatus {
	EntityStatusLoading,
	EntityStatusReady,
	EntityStatusDestroyed,
};

class SUEDE_API IEntity : virtual public IObject {
public:
	virtual void SetStatus(EntityStatus value) = 0;
	virtual EntityStatus GetStatus() const = 0;

	virtual bool GetActive() const = 0;

	virtual void SetActiveSelf(bool value) = 0;
	virtual bool GetActiveSelf() const = 0;

	virtual const std::string& GetTag() const = 0;
	virtual bool SetTag(const std::string& value) = 0;

	virtual std::string GetName() const = 0;
	virtual void SetName(const std::string& value) = 0;

	virtual void Update() = 0;

	virtual void SetTransform(Transform value) = 0;
	virtual Transform GetTransform() const = 0;

	virtual void SetAnimation(Animation value) = 0;
	virtual Animation GetAnimation() = 0;

	/**
	 * @returns bounds measured in the world space.
	 */
	virtual const Bounds& GetBounds() = 0;

	virtual void RecalculateBounds() = 0;

	virtual void SetMesh(Mesh value) = 0;
	virtual Mesh GetMesh() = 0;

	virtual void SetParticleSystem(ParticleSystem value) = 0;
	virtual ParticleSystem GetParticleSystem() = 0;

	virtual void SetRenderer(Renderer value) = 0;
	virtual Renderer GetRenderer() = 0;
};
