#pragma once
#include "object.h"
#include "bounds.h"
#include "transform.h"

SUEDE_DEFINE_OBJECT_POINTER(GameObject);
SUEDE_DECLARE_OBJECT_CREATER(GameObject);

SUEDE_DEFINE_OBJECT_POINTER(Component);

enum {
	RecalculateBoundsFlagsSelf = 1,
	RecalculateBoundsFlagsParent = 2,
	RecalculateBoundsFlagsChildren = 4,
	RecalculateBoundsFlagsAll = -1,
};

class SUEDE_API IGameObject : virtual public IObject {
public:
	virtual bool GetActive() const = 0;

	virtual void SetActiveSelf(bool value) = 0;
	virtual bool GetActiveSelf() const = 0;

	virtual int GetUpdateStrategy() = 0;

	virtual const std::string& GetTag() const = 0;
	virtual bool SetTag(const std::string& value) = 0;

	virtual std::string GetName() const = 0;
	virtual void SetName(const std::string& value) = 0;

	virtual void CullingUpdate() = 0;
	virtual void RenderingUpdate() = 0;

	virtual Transform GetTransform() = 0;
	/**
	 * @returns bounds measured in the world space.
	 */
	virtual const Bounds& GetBounds() = 0;
	virtual void RecalculateBounds(int flags = RecalculateBoundsFlagsAll) = 0;

	virtual void RecalculateUpdateStrategy() = 0;

public:	// Component system.
	template <class T>
	std::shared_ptr<T> AddComponent();

	template <class T> std::shared_ptr<T> GetComponent();
	template <class T> std::vector<std::shared_ptr<T>> GetComponents();

private:
	virtual Component AddComponentHelper(suede_guid type) = 0;
	virtual Component AddComponentHelper(Component component) = 0;

	virtual Component GetComponentHelper(suede_guid type) = 0;
	virtual std::vector<Component> GetComponentsHelper(suede_guid type) = 0;
};

template <class T>
std::shared_ptr<T> IGameObject::AddComponent() {
	return suede_dynamic_cast<std::shared_ptr<T>>(AddComponentHelper(std::shared_ptr<T>(new T)));
}

template <class T>
std::shared_ptr<T> IGameObject::GetComponent() {
	return suede_dynamic_cast<std::shared_ptr<T>>(GetComponentHelper(T::GetTypeID()));
}

template <class T>
std::vector<std::shared_ptr<T>> IGameObject::GetComponents() {
	std::vector<std::shared_ptr<T>> components;
	for (Component component : GetComponentsHelper(T::GetTypeID())) {
		components.push_back(suede_dynamic_cast<std::shared_ptr<T>>(component));
	}

	return components;
}

#include "mesh.h"
#include "light.h"
#include "camera.h"
#include "renderer.h"
#include "animation.h"
#include "particlesystem.h"

#define RTTI_CLASS_SPECIALIZATION(T) \
	template <> \
	inline std::shared_ptr<I ## T> IGameObject::AddComponent() { \
		return suede_dynamic_cast<T>(AddComponentHelper(I ## T::GetTypeID())); \
	}

RTTI_CLASS_SPECIALIZATION(MeshFilter)
RTTI_CLASS_SPECIALIZATION(Camera)
RTTI_CLASS_SPECIALIZATION(MeshRenderer)
RTTI_CLASS_SPECIALIZATION(SkinnedMeshRenderer)
RTTI_CLASS_SPECIALIZATION(ParticleRenderer)
RTTI_CLASS_SPECIALIZATION(Animation)
RTTI_CLASS_SPECIALIZATION(Transform)
RTTI_CLASS_SPECIALIZATION(PointLight)
RTTI_CLASS_SPECIALIZATION(DirectionalLight)
RTTI_CLASS_SPECIALIZATION(SpotLight)
