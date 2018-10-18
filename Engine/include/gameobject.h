#pragma once
#include "object.h"
#include "bounds.h"
#include "transform.h"

SUEDE_DEFINE_OBJECT_POINTER(GameObject);
SUEDE_DECLARE_OBJECT_CREATER(GameObject);

enum {
	RecalculateBoundsFlagsSelf = 1,
	RecalculateBoundsFlagsParent = 2,
	RecalculateBoundsFlagsChildren = 4,
	RecalculateBoundsFlagsAll = -1,
};

class SUEDE_API IGameObject : public IObject {
	SUEDE_DECLARE_IMPLEMENTATION(GameObject)

public:
	IGameObject();

public:
	bool GetActive() const;

	void SetActiveSelf(bool value);
	bool GetActiveSelf() const;

	int GetUpdateStrategy();

	const std::string& GetTag() const;
	bool SetTag(const std::string& value);

	std::string GetName() const;
	void SetName(const std::string& value);

	void CullingUpdate();
	void RenderingUpdate();

	Transform GetTransform();

	/**
	 * @returns bounds measured in the world space.
	 */
	const Bounds& GetBounds();
	void RecalculateBounds(int flags = RecalculateBoundsFlagsAll);

	void RecalculateUpdateStrategy();

public:	// Component system.
	template <class T> 
	std::shared_ptr<T> AddComponent();
	template <class T> std::shared_ptr<T> GetComponent();
	template <class T> std::vector<std::shared_ptr<T>> GetComponents();

	Component AddComponent(Component component);

	Component AddComponent(const char* name);

	Component GetComponent(suede_guid guid);
	Component GetComponent(const char* name);

	/**
	 * @param guid pass 0 to get all components.
	 */
	std::vector<Component> GetComponents(suede_guid guid);

	/**
	* @param guid pass "" to get all components.
	*/
	std::vector<Component> GetComponents(const char* name);

private:
	Component AddComponent(suede_guid guid);
};

template <class T>
std::shared_ptr<T> IGameObject::AddComponent() {
	return suede_dynamic_cast<std::shared_ptr<T>>(AddComponent(std::make_shared<T>()));
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
