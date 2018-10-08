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
	template <class T, class... Args> 
	std::shared_ptr<T> AddComponent(Args... args);
	template <class T> std::shared_ptr<T> GetComponent();
	template <class T> std::vector<std::shared_ptr<T>> GetComponents();

	virtual Component AddComponent(suede_guid guid) = 0;
	virtual Component AddComponent(Component component) = 0;

	virtual Component GetComponent(suede_guid guid) = 0;

	/**
	 * @param guid pass 0 to get all components.
	 */
	virtual std::vector<Component> GetComponents(suede_guid guid) = 0;
};

template <class T, class... Args>
std::shared_ptr<T> IGameObject::AddComponent(Args... args) {
	return suede_dynamic_cast<std::shared_ptr<T>>(AddComponent(std::make_shared<T>(args...)));
}

template <class T>
std::shared_ptr<T> IGameObject::GetComponent() {
	return suede_dynamic_cast<std::shared_ptr<T>>(GetComponent(T::GetComponentGUID()));
}

template <class T>
std::vector<std::shared_ptr<T>> IGameObject::GetComponents() {
	std::vector<std::shared_ptr<T>> components;
	for (Component component : GetComponents(T::GetComponentGUID())) {
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

#define SUEDE_COMPONENT_SPECIALIZATION(T) \
	template <> \
	inline std::shared_ptr<T::element_type> IGameObject::AddComponent() { \
		return suede_dynamic_cast<T>(AddComponent(T::element_type::GetComponentGUID())); \
	}

SUEDE_COMPONENT_SPECIALIZATION(Transform)
SUEDE_COMPONENT_SPECIALIZATION(Light)
SUEDE_COMPONENT_SPECIALIZATION(Camera)
SUEDE_COMPONENT_SPECIALIZATION(TextMesh)
SUEDE_COMPONENT_SPECIALIZATION(MeshFilter)
SUEDE_COMPONENT_SPECIALIZATION(MeshRenderer)
SUEDE_COMPONENT_SPECIALIZATION(SkinnedMeshRenderer)
SUEDE_COMPONENT_SPECIALIZATION(ParticleRenderer)
SUEDE_COMPONENT_SPECIALIZATION(Animation)
SUEDE_COMPONENT_SPECIALIZATION(ParticleSystem)
